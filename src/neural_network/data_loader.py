import numpy as np
from pathlib import Path
import struct

BOARD_SIZE = 19
NUM_CHANNELS = 7


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
            winner = struct.unpack('i', file.read(4))[0]  # not used, OK to keep

            samples = []
            for _ in range(num_samples):
                board = np.frombuffer(
                    file.read(BOARD_SIZE * BOARD_SIZE * 4),
                    dtype=np.uint32
                ).reshape((BOARD_SIZE, BOARD_SIZE))

                policy = np.frombuffer(
                    file.read(BOARD_SIZE * BOARD_SIZE * 4),
                    dtype=np.float32
                ).reshape((BOARD_SIZE, BOARD_SIZE))

                value = struct.unpack('f', file.read(4))[0]
                current_player = struct.unpack('i', file.read(4))[0]
                captured_black = struct.unpack('I', file.read(4))[0]  # ignored
                captured_white = struct.unpack('I', file.read(4))[0]  # ignored
                move_number = struct.unpack('i', file.read(4))[0]

                samples.append({
                    'board': board,
                    'policy': policy,
                    'value': value,
                    'current_player': current_player,
                    'move_number': move_number
                })

        return samples

    def setup_tensor(self, board, current_player, move_number):
        """
        AlphaGo Zero–style feature planes:

        0: current player's stones
        1: opponent stones
        2: empty cells
        3: player-to-move (all 1s if black, else 0s)
        4: move number (normalized)
        5: bias plane (all 1s)
        6: unused (zeros, reserved)
        """

        planes = np.zeros((NUM_CHANNELS, BOARD_SIZE, BOARD_SIZE), dtype=np.float32)

        planes[0] = (board == current_player)
        planes[1] = (board == -current_player)
        planes[2] = (board == 0)

        if current_player == 1:
            planes[3].fill(1.0)

        planes[4].fill(move_number / (BOARD_SIZE * BOARD_SIZE))
        planes[5].fill(1.0)
        # planes[6] left as zeros

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
                # Convert board encoding: {0,1,2} → {0,1,-1}
                board = sample['board'].astype(np.int8)
                board[board == 2] = -1

                planes = self.setup_tensor(
                    board,
                    sample['current_player'],
                    sample['move_number']
                )
                all_boards.append(planes)

                # Policy: flatten + normalize
                policy = sample['policy'].astype(np.float32).flatten()
                policy_sum = policy.sum()
                if policy_sum > 0:
                    policy /= policy_sum
                all_policies.append(policy)

                value_clamped = np.clip(sample['value'], -1.0, 1.0)
                all_values.append(value_clamped)

        if not all_boards:
            raise ValueError("No training data loaded.")

        X = np.asarray(all_boards, dtype=np.float32)           # (N, 7, 19, 19)
        policy_y = np.asarray(all_policies, dtype=np.float32) # (N, 361)
        value_y = np.asarray(all_values, dtype=np.float32).reshape(-1, 1)

        return X, policy_y, value_y
