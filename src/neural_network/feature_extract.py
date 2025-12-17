import torch
import torch.nn as nn
import torch.nn.functional as F


class ResidualBlock(nn.Module):
    """Residual block with two conv layers"""
    def __init__(self, channels):
        super().__init__()
        self.conv1 = nn.Conv2d(channels, channels, kernel_size=3, padding=1)
        self.bn1 = nn.BatchNorm2d(channels)
        self.conv2 = nn.Conv2d(channels, channels, kernel_size=3, padding=1)
        self.bn2 = nn.BatchNorm2d(channels)
    
    def forward(self, x):
        residual = x
        x = F.relu(self.bn1(self.conv1(x)))
        x = self.bn2(self.conv2(x))
        x = F.relu(x + residual)
        return x


class GomokuNet(nn.Module):
    """
    Dual-head network for Gomoku:
    - Policy head: probability distribution over 361 moves
    - Value head: win probability [-1, 1]
    """
    def __init__(self, input_channels=7, num_res_blocks=6, channels=128, dropout=0.3):
        super().__init__()
        
        # Initial conv block
        self.conv_input = nn.Conv2d(input_channels, channels, kernel_size=3, padding=1)
        self.bn_input = nn.BatchNorm2d(channels)
        
        # Residual tower
        self.res_blocks = nn.ModuleList([
            ResidualBlock(channels) for _ in range(num_res_blocks)
        ])
        
        # Dropout for regularization
        self.dropout = nn.Dropout(dropout)
        
        # Policy head
        self.policy_conv = nn.Conv2d(channels, 32, kernel_size=1)
        self.policy_bn = nn.BatchNorm2d(32)
        self.policy_fc = nn.Linear(32 * 19 * 19, 361)
        
        # Value head
        self.value_conv = nn.Conv2d(channels, 1, kernel_size=1)
        self.value_bn = nn.BatchNorm2d(1)
        self.value_fc1 = nn.Linear(19 * 19, 128)
        self.value_fc2 = nn.Linear(128, 1)
    
    def forward(self, x):
        # Input block
        x = F.relu(self.bn_input(self.conv_input(x)))
        
        # Residual tower
        for block in self.res_blocks:
            x = block(x)
        
        # Policy head
        policy = F.relu(self.policy_bn(self.policy_conv(x)))
        policy = policy.view(-1, 32 * 19 * 19)
        policy = self.dropout(policy)
        policy = self.policy_fc(policy)  # Raw logits (use softmax for probabilities)
        
        # Value head
        value = F.relu(self.value_bn(self.value_conv(x)))
        value = value.view(-1, 19 * 19)
        value = F.relu(self.value_fc1(value))
        value = self.dropout(value)
        value = torch.tanh(self.value_fc2(value))  # Output in [-1, 1]
        
        return policy, value


def count_parameters(model):
    return sum(p.numel() for p in model.parameters() if p.requires_grad)
