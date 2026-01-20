#!/usr/bin/env python3
"""
Diagnostic script to analyze training data quality
"""
import numpy as np
from pathlib import Path
import sys

sys.path.insert(0, '/home/ncoursol/Documents/Goban/src/neural_network')
from data_loader import DataLoader

def analyze_data(num_games=10):
    """Analyze the first N games to understand data distribution"""
    data_loader = DataLoader('generated_games')
    
    all_policies = []
    all_values = []
    all_policy_max = []
    
    for game_id in range(num_games):
        try:
            samples = data_loader.load_game(game_id)
            for sample in samples:
                policy = sample['policy'].flatten()
                value = sample['value']
                
                all_policies.append(policy)
                all_values.append(value)
                all_policy_max.append(np.max(policy))
        except FileNotFoundError:
            print(f"Game {game_id} not found, skipping")
            continue
    
    all_policies = np.array(all_policies)
    all_values = np.array(all_values)
    all_policy_max = np.array(all_policy_max)
    
    print("\n" + "="*60)
    print("TRAINING DATA ANALYSIS")
    print("="*60)
    
    print(f"\nTotal samples loaded: {len(all_values)}")
    
    # Policy analysis
    print("\n--- POLICY HEAD ---")
    print(f"Policy shape: {all_policies.shape}")
    print(f"Policy sum per position (should be ~1.0): min={np.min(all_policies.sum(axis=1)):.4f}, max={np.max(all_policies.sum(axis=1)):.4f}, mean={np.mean(all_policies.sum(axis=1)):.4f}")
    print(f"Policy max per position: min={np.min(all_policy_max):.4f}, max={np.max(all_policy_max):.4f}, mean={np.mean(all_policy_max):.4f}")
    print(f"Zero policies: {np.sum(all_policies == 0)} / {all_policies.size} ({100*np.sum(all_policies == 0)/all_policies.size:.2f}%)")
    
    # Value analysis
    print("\n--- VALUE HEAD ---")
    print(f"Value range: [{np.min(all_values):.4f}, {np.max(all_values):.4f}]")
    print(f"Value mean: {np.mean(all_values):.4f}")
    print(f"Value std: {np.std(all_values):.4f}")
    print(f"Value distribution:")
    print(f"  -1.0 (losses): {np.sum(all_values == -1.0)} ({100*np.sum(all_values == -1.0)/len(all_values):.1f}%)")
    print(f"   0.0 (draws): {np.sum(all_values == 0.0)} ({100*np.sum(all_values == 0.0)/len(all_values):.1f}%)")
    print(f"  +1.0 (wins): {np.sum(all_values == 1.0)} ({100*np.sum(all_values == 1.0)/len(all_values):.1f}%)")
    
    # Check for intermediate values
    non_binary = all_values[(all_values != -1.0) & (all_values != 0.0) & (all_values != 1.0)]
    if len(non_binary) > 0:
        print(f"  Other values: {len(non_binary)} samples with values {np.unique(non_binary)}")
    
    print("\n" + "="*60)

if __name__ == '__main__':
    num_games = 20 if len(sys.argv) < 2 else int(sys.argv[1])
    analyze_data(num_games)
