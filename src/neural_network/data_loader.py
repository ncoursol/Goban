import numpy as np
from pathlib import Path
import struct

class DataLoader:
    def __init__(self, path='generated_games'):
        self.path = Path(path) / "training"
    
    def load_game(self, game_id):
        game_path = self.path / f"game_{game_id:05d}.dat"

        if not game_path.exists():
            raise FileNotFoundError(f"Game file {game_path} not found.")

        with open(game_path, mode='rb') as file:
            # Header
            num_samples = struct.unpack('i', file.read(4))[0]
            winner = struct.unpack('i', file.read(4))[0]

            # Data
            samples = []
            for _ in range(num_samples):
                board = np.frombuffer(file.read(19 * 19 * 4), dtype=np.uint32).reshape((19, 19))
                policy = np.frombuffer(file.read(19 * 19 * 4), dtype=np.float32).reshape((19, 19))
                value = struct.unpack('f', file.read(4))[0]
                current_player = struct.unpack('i', file.read(4))[0]
                captured_black = struct.unpack('I', file.read(4))[0]
                captured_white = struct.unpack('I', file.read(4))[0]
                move_number = struct.unpack('i', file.read(4))[0]

                samples.append({
                    'board': board,
                    'policy': policy,
                    'value': value,
                    'current_player': current_player,
                    'captured_black': captured_black,
                    'captured_white': captured_white,
                    'move_number': move_number
                })

        print(f"Loading game from {game_path}")
        return samples

    def setup_tensor(self, board, current_player, captured_black, captured_white, move_number):
        """
        0: Current player's stones (1 where current player has stone)
        1: Opponent's stones (1 where opponent has stone)
        2: Empty positions (1 where empty)
        3: Current player indicator (all 1s if black, all 0s if white)
        4: Captured black stones (normalized)
        5: Captured white stones (normalized)
        6: Move number (normalized)
        
        Returns: numpy array of shape (7, 19, 19)
        """
        planes = np.zeros((7, 19, 19), dtype=np.float32)
        
        if current_player == 1:
            planes[0] = (board == 1).astype(np.float32)
            planes[1] = (board == 2).astype(np.float32)
            my_captures = captured_white
            opp_captures = captured_black
        else:
            planes[0] = (board == 2).astype(np.float32)
            planes[1] = (board == 1).astype(np.float32)
            my_captures = captured_black
            opp_captures = captured_white

        planes[2] = (board == 0).astype(np.float32)
        planes[3] = np.full((19, 19), current_player - 1, dtype=np.float32)
        planes[4] = np.full((19, 19), my_captures / 10.0, dtype=np.float32)
        planes[5] = np.full((19, 19), opp_captures / 10.0, dtype=np.float32)
        planes[6] = np.full((19, 19), move_number / 361.0, dtype=np.float32)
        
        return planes

    def prepare_training_data(self, game_ids):
        all_boards = []
        all_policies = []
        all_values = []
        
        for game_id in game_ids:
            try:
                samples = self.load_game(game_id)
            except FileNotFoundError:
                continue
            
            for sample in samples:
                # Convert board to input planes
                planes = self.setup_tensor(sample['board'], sample['current_player'], sample['captured_black'], sample['captured_white'], sample['move_number'])
                all_boards.append(planes)
                
                # Flatten policy to (361,) for cross-entropy loss
                policy = sample['policy'].flatten()
                all_policies.append(policy)
                
                # Value target
                all_values.append(sample['value'])
        
        if not all_boards:
            raise ValueError("No training data loaded")
        
        X = np.array(all_boards, dtype=np.float32)           # (N, 7, 19, 19)
        policy_y = np.array(all_policies, dtype=np.float32)  # (N, 361)
        value_y = np.array(all_values, dtype=np.float32).reshape(-1, 1)  # (N, 1)
        
        return X, policy_y, value_y