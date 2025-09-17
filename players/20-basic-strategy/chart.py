#!/usr/bin/env python3
"""
Blackjack Strategy HTML Generator

Converts blackjack strategy text files into nicely formatted HTML tables with color coding.
Supports hard hands, soft hands, and pair splitting decisions.
"""

import re
import argparse
from pathlib import Path


class BlackjackStrategyGenerator:
    def __init__(self):
        self.html_template = self._get_html_template()
        self.sections = {
            'h': {'title': 'Hard Hands (No Ace or Ace counted as 1)', 'data': []},
            's': {'title': 'Soft Hands (Ace counted as 11)', 'data': []},
            'p': {'title': 'Pairs (Split Decisions)', 'data': []}
        }
    
    def parse_strategy_file(self, file_path):
        """Parse the strategy text file and extract data for each section."""
        with open(file_path, 'r') as f:
            content = f.read()
        
        # Reset sections
        for section in self.sections.values():
            section['data'] = []
        
        lines = content.strip().split('\n')
        current_section = None
        dealer_cards = []
        
        for line in lines:
            line = line.strip()
            if not line or line.startswith('#'):
                # Check if it's a header line with dealer cards
                if line.startswith('#') and any(c.isalnum() for c in line):
                    # Extract dealer cards from header
                    cards = line.split()[1:]  # Skip the '#'
                    if cards:
                        dealer_cards = cards
                continue
            
            # Parse hand data
            parts = line.split()
            if len(parts) >= 2:
                hand = parts[0]
                actions = parts[1:]
                
                # Determine section based on hand prefix
                if hand.startswith('h'):
                    current_section = 'h'
                    hand_label = hand[1:]  # Remove 'h' prefix
                elif hand.startswith('s'):
                    current_section = 's'
                    # Convert soft hand notation
                    soft_value = int(hand[1:])
                    ace_companion = soft_value - 11
                    if ace_companion == 9:
                        hand_label = 'A,9'
                    elif ace_companion == 8:
                        hand_label = 'A,8'
                    else:
                        hand_label = f'A,{ace_companion}'
                elif hand.startswith('p'):
                    current_section = 'p'
                    # Convert pair notation
                    card = hand[1:]
                    if card == 'A':
                        hand_label = 'A,A'
                    elif card == 'T':
                        hand_label = 'T,T'
                    else:
                        hand_label = f'{card},{card}'
                else:
                    continue
                
                if current_section and len(actions) == len(dealer_cards):
                    self.sections[current_section]['data'].append({
                        'hand': hand_label,
                        'actions': actions
                    })
        
        return dealer_cards
    
    def get_css_class(self, action, section_type):
        """Get CSS class for the action based on section type."""
        action = action.lower()
        
        if section_type == 'p':  # Pairs section
            return 'split-yes' if action == 'y' else 'split-no'
        else:  # Hard/Soft hands
            if action == 'h':
                return 'hit'
            elif action == 's':
                return 'stand'
            elif action == 'd':
                return 'double'
            else:
                return 'hit'  # Default
    
    def get_action_display(self, action, section_type):
        """Get display text for the action."""
        action = action.upper()
        
        if section_type == 'p':  # Pairs section
            return action  # Y or N
        else:  # Hard/Soft hands
            return action  # H, S, or D
    
    def generate_table_html(self, section_type, section_data, dealer_cards):
        """Generate HTML for a single table section."""
        if not section_data['data']:
            return ""
        
        # Sort data by hand value for better presentation
        if section_type == 'h':  # Hard hands - sort by numeric value descending
            section_data['data'].sort(key=lambda x: int(x['hand']), reverse=True)
        elif section_type == 's':  # Soft hands - sort by second card descending
            section_data['data'].sort(key=lambda x: int(x['hand'].split(',')[1]) if x['hand'].split(',')[1].isdigit() else 9, reverse=True)
        elif section_type == 'p':  # Pairs - custom order
            pair_order = ['A,A', 'T,T', '9,9', '8,8', '7,7', '6,6', '5,5', '4,4', '3,3', '2,2']
            section_data['data'].sort(key=lambda x: pair_order.index(x['hand']) if x['hand'] in pair_order else 99)
        
        html = f"""
        <div class="section">
            <div class="section-title">{section_data['title']}</div>
            <table>
                <thead>
                    <tr>
                        <th>{'Pair' if section_type == 'p' else 'Hand'}</th>"""
        
        # Add dealer card headers
        for card in dealer_cards:
            html += f'<th>{card}</th>'
        
        html += """
                    </tr>
                </thead>
                <tbody>"""
        
        # Add data rows
        for hand_data in section_data['data']:
            html += f'<tr><td class="hand-label">{hand_data["hand"]}</td>'
            
            for action in hand_data['actions']:
                css_class = self.get_css_class(action, section_type)
                display_text = self.get_action_display(action, section_type)
                html += f'<td class="{css_class}">{display_text}</td>'
            
            html += '</tr>'
        
        html += """
                </tbody>
            </table>
        </div>"""
        
        return html
    
    def generate_html(self, file_path, output_path=None, title="Blackjack Basic Strategy Chart"):
        """Generate complete HTML file from strategy text file."""
        dealer_cards = self.parse_strategy_file(file_path)
        
        if not dealer_cards:
            raise ValueError("Could not parse dealer cards from file")
        
        # Generate tables for each section
        tables_html = ""
        for section_type in ['h', 's', 'p']:
            if self.sections[section_type]['data']:
                tables_html += self.generate_table_html(section_type, self.sections[section_type], dealer_cards)
        
        # Replace placeholders in template
        html_content = self.html_template.replace('{{TITLE}}', title)
        html_content = html_content.replace('{{TABLES}}', tables_html)
        
        # Determine output file path
        if output_path is None:
            input_path = Path(file_path)
            output_path = input_path.with_suffix('.html')
        
        # Write HTML file
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html_content)
        
        print(f"Generated HTML strategy chart: {output_path}")
        return str(output_path)
    
    def _get_html_template(self):
        """Return the HTML template with placeholders."""
        return '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{{TITLE}}</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }
        
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 30px;
        }
        
        .section {
            margin-bottom: 30px;
        }
        
        .section-title {
            font-size: 18px;
            font-weight: bold;
            margin-bottom: 10px;
            padding: 10px;
            background: #2c3e50;
            color: white;
            border-radius: 5px;
            text-align: center;
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 20px;
            font-size: 14px;
            font-weight: bold;
        }
        
        th, td {
            border: 2px solid #34495e;
            text-align: center;
            padding: 10px;
            font-weight: bold;
        }
        
        th {
            background-color: #34495e;
            color: white;
            font-size: 16px;
        }
        
        .hand-label {
            background-color: #34495e;
            color: white;
            font-weight: bold;
            width: 60px;
        }
        
        /* Action colors */
        .hit {
            background-color: #e74c3c;
            color: white;
        }
        
        .stand {
            background-color: #27ae60;
            color: white;
        }
        
        .double {
            background-color: #f39c12;
            color: white;
        }
        
        .split-yes {
            background-color: #3498db;
            color: white;
        }
        
        .split-no {
            background-color: #95a5a6;
            color: white;
        }
        
        .legend {
            display: flex;
            justify-content: center;
            gap: 20px;
            margin-bottom: 20px;
            flex-wrap: wrap;
        }
        
        .legend-item {
            display: flex;
            align-items: center;
            gap: 5px;
        }
        
        .legend-color {
            width: 20px;
            height: 20px;
            border: 2px solid #34495e;
            border-radius: 3px;
        }
        
        .legend-text {
            font-weight: bold;
            color: #333;
        }
        
        @media (max-width: 768px) {
            body {
                margin: 10px;
            }
            
            .container {
                padding: 10px;
            }
            
            table {
                font-size: 12px;
            }
            
            th, td {
                padding: 6px;
            }
            
            .legend {
                gap: 10px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>{{TITLE}}</h1>
        
        <div class="legend">
            <div class="legend-item">
                <div class="legend-color hit"></div>
                <span class="legend-text">Hit (H)</span>
            </div>
            <div class="legend-item">
                <div class="legend-color stand"></div>
                <span class="legend-text">Stand (S)</span>
            </div>
            <div class="legend-item">
                <div class="legend-color double"></div>
                <span class="legend-text">Double (D)</span>
            </div>
            <div class="legend-item">
                <div class="legend-color split-yes"></div>
                <span class="legend-text">Split (Y)</span>
            </div>
            <div class="legend-item">
                <div class="legend-color split-no"></div>
                <span class="legend-text">No Split (N)</span>
            </div>
        </div>
        
        {{TABLES}}
    </div>
</body>
</html>'''


def main():
    parser = argparse.ArgumentParser(description='Generate HTML blackjack strategy charts from text files')
    parser.add_argument('input_file', help='Input strategy text file')
    parser.add_argument('-o', '--output', help='Output HTML file (default: input_file.html)')
    parser.add_argument('-t', '--title', default='Blackjack Basic Strategy Chart', 
                       help='Title for the HTML page')
    
    args = parser.parse_args()
    
    try:
        generator = BlackjackStrategyGenerator()
        output_file = generator.generate_html(args.input_file, args.output, args.title)
        print(f"Successfully generated: {output_file}")
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    exit(main())


# Example usage as a module:
"""
from blackjack_strategy_generator import BlackjackStrategyGenerator

generator = BlackjackStrategyGenerator()

# Generate HTML from strategy file
generator.generate_html(
    'strategy.txt',
    'my_strategy.html', 
    'Single Deck Basic Strategy'
)

# Or use with different rule sets:
generator.generate_html(
    'double_deck_strategy.txt',
    'double_deck.html',
    'Double Deck Basic Strategy'
)
"""
