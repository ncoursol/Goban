#!/usr/bin/env python3
"""
Visualize generated training games.
Displays board states and statistics.
"""

import json
import sys
from pathlib import Path


def print_board(board, move_x=None, move_y=None):
    """Print a 19x19 board with optional highlighted move."""
    print("   ", end="")
    for j in range(19):
        print(f"{j:2}", end=" ")
    print()
    
    for i in range(19):
        print(f"{i:2} ", end="")
        for j in range(19):
            cell = board[i][j]
            highlight = (i == move_x and j == move_y)
            
            if highlight:
                print("\033[93m", end="")  # Yellow highlight
            
            if cell == 0:
                print(" .", end=" ")
            elif cell == 1:
                print(" ○", end=" ")  # Black
            elif cell == 2:
                print(" ●", end=" ")  # White
            else:
                print(" ?", end=" ")
            
            if highlight:
                print("\033[0m", end="")
        print()


def visualize_game(game_path):
    """Visualize a single game."""
    with open(game_path, 'r') as f:
        game = json.load(f)
    
    print("\n" + "="*70)
    print(f"Game {game['game_id']}")
    print("="*70)
    print(f"Winner: Player {game['winner'] + 1}" if game['winner'] < 2 else "Draw")
    print(f"Total moves: {game['total_moves']}")
    print(f"Captured - Black: {game['captured_black']}, White: {game['captured_white']}")
    print(f"Training samples: {game['num_training_samples']}")
    print()
    
    # Show first few moves
    print("First 10 moves:")
    for move in game['moves'][:10]:
        player_name = "Black" if move['player'] == 0 else "White"
        print(f"  Move {move['move']:3}: {player_name:5} plays at ({move['x']:2}, {move['y']:2})")
    
    if len(game['moves']) > 10:
        print(f"  ... ({len(game['moves']) - 10} more moves)")
    
    print("\nFinal board state:")
    print_board(game['final_board'])


def show_statistics(data_dir):
    """Show statistics about all generated games."""
    games_dir = Path(data_dir) / "games"
    game_files = sorted(games_dir.glob("game_*.json"))
    
    if not game_files:
        print(f"No games found in {games_dir}")
        return
    
    print(f"\n{'='*70}")
    print(f"Dataset Statistics - {len(game_files)} games")
    print(f"{'='*70}\n")
    
    total_samples = 0
    total_moves = 0
    min_game = 1000
    id_min = 0
    winners = {"Player 1": 0, "Player 2": 0, "Draw": 0}
    move_lengths = []
    
    for game_file in game_files:
        with open(game_file, 'r') as f:
            game = json.load(f)
        
        total_samples += game['num_training_samples']
        total_moves += game['total_moves']
        move_lengths.append(game['total_moves'])
        if game['total_moves'] < min_game:
            min_game = game['total_moves']
            id_min = game['game_id']
        
        if game['winner'] == 0:
            winners["Player 1"] += 1
        elif game['winner'] == 1:
            winners["Player 2"] += 1
        else:
            winners["Draw"] += 1
    
    print(f"Total games: {len(game_files)}")
    print(f"Total training samples: {total_samples}")
    print(f"Average samples per game: {total_samples / len(game_files):.1f}")
    print(f"Average moves per game: {total_moves / len(game_files):.1f}")
    print(f"Shortest game: {min(move_lengths)} moves")
    print(f"Longest game: {max(move_lengths)} moves")
    print(f"Shortest game id: {id_min}")
    print()
    
    print("Win distribution:")
    total_games = len(game_files)
    for player, count in winners.items():
        percentage = (count / total_games) * 100
        bar = "█" * int(percentage / 2)
        print(f"  {player:10}: {count:4} ({percentage:5.1f}%) {bar}")
    
    print("\n" + "="*70 + "\n")


def main():
    if len(sys.argv) < 2:
        print("Usage:")
        print(f"  {sys.argv[0]} stats [data_dir]           - Show statistics")
        print(f"  {sys.argv[0]} show <game_id> [data_dir]  - Show specific game")
        print()
        print("Default data_dir: generated_games")
        sys.exit(1)
    
    command = sys.argv[1]
    
    if command == "stats":
        data_dir = sys.argv[2] if len(sys.argv) > 2 else "generated_games"
        show_statistics(data_dir)
    
    elif command == "show":
        if len(sys.argv) < 3:
            print("Error: Game ID required")
            print(f"Usage: {sys.argv[0]} show <game_id> [data_dir]")
            sys.exit(1)
        
        game_id = int(sys.argv[2])
        data_dir = sys.argv[3] if len(sys.argv) > 3 else "generated_games"
        
        game_path = Path(data_dir) / "games" / f"game_{game_id:05d}.json"
        if not game_path.exists():
            print(f"Error: Game {game_id} not found at {game_path}")
            sys.exit(1)
        
        visualize_game(game_path)
    
    else:
        print(f"Unknown command: {command}")
        print("Use 'stats' or 'show'")
        sys.exit(1)


if __name__ == "__main__":
    main()
