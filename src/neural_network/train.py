import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader as TorchDataLoader
import numpy as np
from pathlib import Path
import argparse
import time
from datetime import datetime
from tqdm import tqdm
import matplotlib.pyplot as plt
import seaborn as sns

from network import GomokuNet, count_parameters
from data_loader import DataLoader


class GomokuDataset(Dataset):
    """PyTorch Dataset wrapper for Gomoku training data"""
    def __init__(self, X, policy_y, value_y, augment=False):
        self.X = torch.from_numpy(X)
        self.policy_y = torch.from_numpy(policy_y)
        self.value_y = torch.from_numpy(value_y)
        self.augment = augment
    
    def __len__(self):
        return len(self.X)
    
    def __getitem__(self, idx):
        x = self.X[idx].clone()
        policy = self.policy_y[idx].clone()
        value = self.value_y[idx]

        if self.augment:
            x, policy = self._apply_augmentation(x, policy)
        
        return x, policy, value
    
    def _apply_augmentation(self, x, policy):
        """Apply random rotation and flip (D4 symmetry)"""
        policy_2d = policy.view(19, 19)
        
        k = torch.randint(0, 4, (1,)).item()
        if k > 0:
            x = torch.rot90(x, k, dims=[1, 2])
            policy_2d = torch.rot90(policy_2d, k, dims=[0, 1])
        
        if torch.rand(1).item() > 0.5:
            x = torch.flip(x, dims=[2])
            policy_2d = torch.flip(policy_2d, dims=[1])
        
        return x, policy_2d.flatten()


class GomokuTrainer:
    def __init__(self, model, device='cuda', lr=0.001, weight_decay=1e-4, value_weight=0.1):
        self.model = model.to(device)
        self.device = device
        self.optimizer = optim.AdamW(
            model.parameters(),
            lr=lr,
            weight_decay=weight_decay
        )

        # Loss functions
        self.value_loss_fn = nn.MSELoss()
        self.value_weight = value_weight
        
        # Training history
        self.history = {
            'train_loss': [],
            'train_policy_loss': [],
            'train_value_loss': [],
            'val_loss': [],
            'val_policy_loss': [],
            'val_value_loss': []
        }
    
    def compute_loss(self, X, policy_logits, value_pred, policy_target, value_target):
        log_probs = torch.log_softmax(policy_logits, dim=1)
        legal_mask = X[:, 2].view(X.size(0), -1)
        log_probs = log_probs + (legal_mask - 1) * 1e10
        policy_loss = -torch.sum(policy_target * log_probs, dim=1).mean()

        value_loss = self.value_loss_fn(value_pred, value_target)
        total_loss = policy_loss + self.value_weight * value_loss
        return total_loss, policy_loss, value_loss
    
    def train_epoch(self, train_loader, epoch, total_epochs):
        self.model.train()
        total_loss = 0
        total_policy_loss = 0
        total_value_loss = 0
        num_batches = 0
        
        pbar = tqdm(train_loader, desc=f"Epoch {epoch}/{total_epochs} [Train]", 
                    leave=False, ncols=100)
        for X, policy_y, value_y in pbar:
            X = X.to(self.device)
            policy_y = policy_y.to(self.device)
            value_y = value_y.to(self.device)
            
            self.optimizer.zero_grad()
            policy_logits, value_pred = self.model(X)
            loss, policy_loss, value_loss = self.compute_loss(
                X, policy_logits, value_pred, policy_y, value_y
            )

            
            loss.backward()
            torch.nn.utils.clip_grad_norm_(self.model.parameters(), max_norm=1.0)
            self.optimizer.step()
            
            total_loss += loss.item()
            total_policy_loss += policy_loss.item()
            total_value_loss += value_loss.item()
            num_batches += 1
            
            pbar.set_postfix({
                'loss': f'{total_loss/num_batches:.4f}',
            })
        
        pbar.close()
        return {
            'loss': total_loss / num_batches,
            'policy_loss': total_policy_loss / num_batches,
            'value_loss': total_value_loss / num_batches,
        }
    
    @torch.no_grad()
    def validate(self, val_loader, epoch, total_epochs):
        self.model.eval()
        total_loss = 0
        total_policy_loss = 0
        total_value_loss = 0
        num_batches = 0
        
        pbar = tqdm(val_loader, desc=f"Epoch {epoch}/{total_epochs} [Valid]", 
                    leave=False, ncols=100)
        for X, policy_y, value_y in pbar:
            X = X.to(self.device)
            policy_y = policy_y.to(self.device)
            value_y = value_y.to(self.device)
            
            policy_logits, value_pred = self.model(X)
            loss, policy_loss, value_loss = self.compute_loss(
                X, policy_logits, value_pred, policy_y, value_y
            )
            
            total_loss += loss.item()
            total_policy_loss += policy_loss.item()
            total_value_loss += value_loss.item()
            num_batches += 1
            
            pbar.set_postfix({
                'loss': f'{total_loss/num_batches:.4f}',
            })
        
        pbar.close()
        return {
            'loss': total_loss / num_batches,
            'policy_loss': total_policy_loss / num_batches,
            'value_loss': total_value_loss / num_batches,
        }
    
    def train(self, train_loader, val_loader=None, epochs=100):
        print(f"Training on {self.device}")
        print(f"Model parameters: {count_parameters(self.model):,}")
        print(f"Training samples: {len(train_loader.dataset)}")
        if val_loader:
            print(f"Validation samples: {len(val_loader.dataset)}")
        print("-" * 60)
        
        for epoch in range(1, epochs + 1):
            start_time = time.time()
            
            train_metrics = self.train_epoch(train_loader, epoch, epochs)
            
            val_metrics = None
            if val_loader:
                val_metrics = self.validate(val_loader, epoch, epochs)
            
            if self.device == 'cuda':
                torch.cuda.synchronize()
            epoch_time = time.time() - start_time
            
            self.history['train_loss'].append(train_metrics['loss'])
            self.history['train_policy_loss'].append(train_metrics['policy_loss'])
            self.history['train_value_loss'].append(train_metrics['value_loss'])
            
            if val_metrics:
                self.history['val_loss'].append(val_metrics['loss'])
                self.history['val_policy_loss'].append(val_metrics['policy_loss'])
                self.history['val_value_loss'].append(val_metrics['value_loss'])

            print(
                f"Epoch {epoch:3d}/{epochs} | "
                f"Train Loss: {train_metrics['loss']:.4f} "
                f"(P: {train_metrics['policy_loss']:.4f}, "
                f"V: {train_metrics['value_loss']:.4f})",
                end=""
            )

            if val_metrics:
                print(
                    f" | Val Loss: {val_metrics['loss']:.4f} "
                    f"(P: {val_metrics['policy_loss']:.4f}, "
                    f"V: {val_metrics['value_loss']:.4f})",
                    end=""
                )

            print(f" | {epoch_time:.1f}s")

        
        print(f"\nTraining complete")
        return self.history
    
    def plot_training_history(self, save_path=None):
        """Plot training metrics"""
        sns.set_style("whitegrid")
        sns.set_palette("husl")
        
        epochs = range(1, len(self.history['train_loss']) + 1)
        fig, axes = plt.subplots(2, 2, figsize=(14, 10))
        fig.suptitle('Training History', fontsize=16, fontweight='bold')
        
        ax1 = axes[0, 0]
        ax1.plot(epochs, self.history['train_loss'], 'b-', label='Train Loss', linewidth=2)
        if self.history['val_loss']:
            ax1.plot(epochs, self.history['val_loss'], 'r-', label='Val Loss', linewidth=2)
        ax1.set_xlabel('Epoch')
        ax1.set_ylabel('Loss')
        ax1.set_title('Total Loss')
        ax1.legend()
        ax1.set_xlim(1, len(epochs))
        
        ax2 = axes[0, 1]
        ax2.plot(epochs, self.history['train_policy_loss'], 'b-', label='Train Policy Loss', linewidth=2)
        if self.history['val_policy_loss']:
            ax2.plot(epochs, self.history['val_policy_loss'], 'r-', label='Val Policy Loss', linewidth=2)
        ax2.set_xlabel('Epoch')
        ax2.set_ylabel('Loss')
        ax2.set_title('Policy Loss')
        ax2.legend()
        ax2.set_xlim(1, len(epochs))
        
        ax3 = axes[1, 0]
        ax3.plot(epochs, self.history['train_value_loss'], 'b-', label='Train Value Loss', linewidth=2)
        if self.history['val_value_loss']:
            ax3.plot(epochs, self.history['val_value_loss'], 'r-', label='Val Value Loss', linewidth=2)
        ax3.set_xlabel('Epoch')
        ax3.set_ylabel('Loss')
        ax3.set_title('Value Loss')
        ax3.legend()
        ax3.set_xlim(1, len(epochs))
        
        plt.tight_layout()
        
        if save_path:
            plt.savefig(save_path, dpi=150, bbox_inches='tight')
            print(f"Training plots saved to {save_path}")
        
        plt.show()


def create_data_loaders(data_loader, game_ids, batch_size=64, val_split=0.1, augment=True):
    """Create train and validation data loaders"""
    print(f"Loading {len(game_ids)} games...")
    X, policy_y, value_y = data_loader.prepare_training_data(game_ids)
    print(f"Total samples: {len(X)}")
    
    indices = np.random.permutation(len(X))
    val_size = int(len(X) * val_split)
    
    val_indices = indices[:val_size]
    train_indices = indices[val_size:]
    
    train_dataset = GomokuDataset(X[train_indices], policy_y[train_indices], value_y[train_indices], augment=augment)
    val_dataset = GomokuDataset(X[val_indices], policy_y[val_indices], value_y[val_indices], augment=False)
    
    train_loader = TorchDataLoader(
        train_dataset,
        batch_size=batch_size,
        shuffle=True,
        num_workers=4,
        pin_memory=True,
        persistent_workers=True
    )

    val_loader = TorchDataLoader(
        val_dataset,
        batch_size=batch_size,
        shuffle=False,
        num_workers=4,
        pin_memory=True,
        persistent_workers=True
    )

    
    return train_loader, val_loader


def main():
    import random

    torch.manual_seed(0)
    np.random.seed(0)
    random.seed(0)

    parser = argparse.ArgumentParser(description='Train Gomoku Neural Network')
    parser.add_argument('--data-path', type=str, default='generated_games',
                        help='Path to generated games directory')
    parser.add_argument('--num-games', type=int, default=100,
                        help='Number of games to use for training')
    parser.add_argument('--epochs', type=int, default=100,
                        help='Number of training epochs')
    parser.add_argument('--batch-size', type=int, default=64,
                        help='Batch size for training')
    parser.add_argument('--lr', type=float, default=0.001,
                        help='Learning rate')
    parser.add_argument('--weight-decay', type=float, default=1e-4,
                        help='Weight decay for regularization')
    parser.add_argument('--value-weight', type=float, default=0.1,
                        help='Weight for value loss relative to policy loss')
    parser.add_argument('--res-blocks', type=int, default=4,
                        help='Number of residual blocks')
    parser.add_argument('--channels', type=int, default=64,
                        help='Number of channels in residual tower')
    parser.add_argument('--no-augment', action='store_true',
                        help='Disable data augmentation')
    parser.add_argument('--no-plot', action='store_true',
                        help='Disable plotting training history')
    parser.add_argument('--device', type=str, default='auto',
                        help='Device to use (cuda/cpu/auto)')
    args = parser.parse_args()
    
    if args.device == 'auto':
        device = 'cuda' if torch.cuda.is_available() else 'cpu'
    else:
        device = args.device
    
    print(f"=" * 60)
    print(f"Gomoku Neural Network Training")
    print(f"Started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"=" * 60)
    
    data_loader = DataLoader(args.data_path)
    game_ids = list(range(args.num_games))
    train_loader, val_loader = create_data_loaders(
        data_loader, game_ids, 
        batch_size=args.batch_size,
        augment=not args.no_augment
    )
    
    model = GomokuNet(
        input_channels=7,
        num_res_blocks=args.res_blocks,
        channels=args.channels,
    )
    
    trainer = GomokuTrainer(
        model, 
        device=device,
        lr=args.lr,
        weight_decay=args.weight_decay,
        value_weight=args.value_weight
    )

    
    history = trainer.train(train_loader, val_loader, epochs=args.epochs)
    
    print(f"\nFinal training loss: {history['train_loss'][-1]:.4f}")
    print(f"Final validation loss: {history['val_loss'][-1]:.4f}")
    print(f"Best validation loss: {min(history['val_loss']):.4f}")
    
    if not args.no_plot:
        trainer.plot_training_history()


if __name__ == '__main__':
    main()
