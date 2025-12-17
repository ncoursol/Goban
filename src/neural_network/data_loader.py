import numpy as np
from pathlib import Path

class DataLoader:
    def __init__(self, path='generated_games'):
        self.path = Path(path) / "training"
    
    def load_game(self, game_id):
        game_path = self.path / f"game_{game_id:05d}.dat"

        if not game_path.exists():
            raise FileNotFoundError(f"Game file {game_path} not found.")
        print(f"Loading game from {game_path}")
        
