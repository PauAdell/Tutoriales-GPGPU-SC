#!/usr/bin/env python3
import subprocess
import re
import os
import time
import sys
import argparse

def run_benchmark(api_name, bin_name, cwd):
    print(f"Running: {bin_name} in {cwd}")
    try:
        # We run from the directory where the binary is
        process = subprocess.Popen([f"./{bin_name}"], cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        # Wait for the process to finish and gather output
        stdout, stderr = process.communicate(timeout=60) # Max timeout for communication
        
        output = stdout
        if stderr:
            print(f"  [!] Stderr output:\n{stderr}")

        results = {}
        
        # Parse timing information using standard labels
        setup_match = re.search(r"Setup time:\s+([\d.]+)", output)
        comp_match = re.search(r"(?:Computation|Operation|Comp\.)\s+time:\s+([\d.]+)", output)
        total_match = re.search(r"Total time:\s+([\d.]+)", output)
        
        # Parse validation result
        valid_match = re.search(r"VALIDATION:\s+(PASSED|FAILED)", output)
        
        if comp_match:
            results['comp'] = float(comp_match.group(1))
        if setup_match:
            results['setup'] = float(setup_match.group(1))
        if total_match:
            results['total'] = float(total_match.group(1))
        
        if valid_match:
            results['validation'] = valid_match.group(1)
        else:
            results['validation'] = "UNKN" # Unknown if not reported
            
        if 'comp' in results:
            return results
        else:
            print(f"  [!] No timing results found for {bin_name}.")
            return None
            
    except subprocess.TimeoutExpired:
        process.kill()
        print(f"  [!] Process {bin_name} timed out.")
        return None
    except Exception as e:
        print(f"  [!] Exception running {bin_name}: {e}")
        return None

def main():
    parser = argparse.ArgumentParser(description='GPGPU Benchmark Script')
    parser.add_argument('--size', type=int, help='Size of the problem (e.g., 1024)')
    args = parser.parse_args()

    if args.size:
        print(f"Recompiling with SIZE={args.size}...")
        try:
            subprocess.run(["make", "clean"], check=True)
            subprocess.run(["make", f"SIZE={args.size}"], check=True)
        except subprocess.CalledProcessError as e:
            print(f"Compilation failed: {e}")
            sys.exit(1)

    # Benchmarks list: (Algorithm, API Name, Binary Name, Subdirectory)
    benchmarks = [
        ("Vector Add", "OpenGL ES2", "vector_add", "01_vector_add/opengl_es2"),
        ("Vector Add", "OpenGL SC2", "vector_add", "01_vector_add/opengl_sc2"),
        ("Vector Add", "Vulkan Headless", "vector_add", "01_vector_add/vulkan_headless"),
        ("Vector Add", "Vulkan GUI", "window_vector_add", "01_vector_add/vulkan_gui"),
        
        ("Mat Mult", "OpenGL ES2", "mat_mult", "02_mat_mult/opengl_es2"),
        ("Mat Mult", "OpenGL SC2", "mat_mult", "02_mat_mult/opengl_sc2"),
        ("Mat Mult", "Vulkan Headless", "mat_mult", "02_mat_mult/vulkan_headless"),
        ("Mat Mult", "OpenMP", "mat_mult", "02_mat_mult/openmp"),
    ]

    results = {}

    for algo, api, bin_name, cwd in benchmarks:
        full_path = os.path.join(cwd, bin_name)
        if not os.path.exists(full_path):
            continue
        
        data = run_benchmark(api, bin_name, cwd)
        if data:
            if algo not in results: results[algo] = []
            results[algo].append((api, data))

    if not results:
        print("\nNo results gathered. Check for errors above.")
        return

    print("\n" + "="*80)
    print("      GPGPU BENCHMARK RESULTS")
    if args.size:
        print(f"      PROBLEM SIZE: {args.size}")
    print("="*80)
    print(f"{'API':<20} | {'Setup (s)':<10} | {'Comp. (s)':<10} | {'Total (s)':<10} | {'Valid'}")
    print("-" * 80)
    
    for algo, res in results.items():
        print(f"\nAlgorithm: {algo}")
        # Sort by computation time
        res.sort(key=lambda x: x[1]['comp'])
        for api, data in res:
            setup = f"{data.get('setup', 0.0):.4f}"
            comp = f"{data.get('comp', 0.0):.4f}"
            total = f"{data.get('total', 0.0):.4f}"
            valid = data.get('validation', 'UNKN')
            
            # Highlight FAILED in color if terminal supports it
            if valid == "FAILED":
                valid = f"\033[91m{valid}\033[0m"
            elif valid == "PASSED":
                valid = f"\033[92m{valid}\033[0m"
                
            print(f"{api:<20} | {setup:>10} | {comp:>10} | {total:>10} | {valid}")
    print("="*80)

if __name__ == "__main__":
    main()
