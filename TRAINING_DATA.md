# Training Data Generation System

Complete system for generating training data from self-play MCTS games to train a neural network (AlphaZero approach).

## Quick Start

### 1. Build
```bash
make game_generator
```

### 2. Generate Games
```bash
./game_generator -n 100        # Generate 100 games
./game_generator -n 1000       # Generate 1000 games
./game_generator -h            # Show help
```

### 3. View Results
```bash
python3 scripts/visualize_games.py stats    # Show statistics
python3 scripts/visualize_games.py show 0   # View specific game
```

### 4. Load Data for Training (requires numpy/torch)
```bash
pip install numpy torch
```

```python
from scripts.load_training_data import TrainingDataLoader

loader = TrainingDataLoader("generated_games")
boards, policies, values = loader.load_all_games()

# boards:   (N, 19, 19) - board states
# policies: (N, 19, 19) - MCTS policy targets (move probabilities)
# values:   (N,)        - game outcomes (-1, 0, +1)
```

## Output Structure

```
generated_games/
├── games/              # JSON game records (human-readable)
│   ├── game_00000.json
│   └── ...
└── training/           # Binary training data (for NN)
    ├── game_00000.dat
    └── ...
```

## Data Format

### JSON Files (Game Records)
Each game contains:
- Game ID and winner
- Move sequence with coordinates
- Final board state
- Captured stones count

### Binary Files (Training Data)
Each position contains:
- **Board state** (19×19): Current stone positions
- **Policy target** (19×19): MCTS visit count distribution (normalized probabilities)
- **Value target** (scalar): Game outcome from current player's view (+1 win, -1 loss, 0 draw)
- **Metadata**: current player, captures, move number

## Training Your Neural Network

Your NN should predict:
1. **Policy** (19×19 softmax): Where MCTS explored
2. **Value** (scalar): Who will win

Example training script provided: `scripts/train_pytorch_example.py`

```bash
python3 scripts/train_pytorch_example.py
```

## Implementation Details

### Files Created
- `include/data_generation.h` - Training data structures
- `src/bot/data_generation.c` - Game generation logic
- `src/bot/game_generator.c` - CLI tool
- `scripts/load_training_data.py` - Python data loader
- `scripts/visualize_games.py` - Visualization tool
- `scripts/train_pytorch_example.py` - PyTorch training example

### Key Features
- Self-play games using MCTS
- Policy targets from MCTS visit counts
- Value targets from game outcomes
- Efficient binary format + human-readable JSON
- Swap2 opening disabled for clean training
- Proper win detection

## Performance

Approximate generation speed (8 CPU cores):
- ~1-2 minutes per game
- 100 games: ~2-3 hours
- 1000 games: ~20-30 hours

## Next Steps

1. Generate initial dataset (100-1000 games)
2. Train neural network on the data
3. Integrate NN into MCTS
4. Generate more games with improved bot
5. Repeat (self-play training loop)
