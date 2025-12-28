#!/usr/bin/env python3
import sys, json, argparse, subprocess
from pathlib import Path

def ensure_venv(outdir: Path):
    venv = outdir / ".venv"
    req = Path(__file__).parent.parent / "requirements.txt"
    if venv.exists():
        return
    print(f"Creating venv: {venv}")
    subprocess.run([sys.executable, "-m", "venv", str(venv)], check=True)
    pip = venv / "bin" / "pip"
    subprocess.run([str(pip), "install", "-q", "-r", str(req)], check=True)
    print(f"Installed deps. Activate: source {venv}/bin/activate")

def create_notebook_cells(csv_path: str) -> list:
    """Create notebook cells for benchmark analysis."""
    cells = []

    # Title cell
    cells.append({
        "cell_type": "markdown",
        "metadata": {},
        "source": [
            "# Benchmark Analysis\n",
            "\n",
            f"Analysis of benchmark results from `{csv_path}`"
        ]
    })

    # Imports cell
    cells.append({
        "cell_type": "code",
        "metadata": {},
        "execution_count": None,
        "outputs": [],
        "source": [
            "import pandas as pd\n",
            "import matplotlib.pyplot as plt\n",
            "import seaborn as sns\n",
            "\n",
            "# Set style\n",
            "plt.style.use('seaborn-v0_8-whitegrid')\n",
            "sns.set_palette('husl')\n",
            "%matplotlib inline"
        ]
    })

    # Load data cell
    cells.append({
        "cell_type": "code",
        "metadata": {},
        "execution_count": None,
        "outputs": [],
        "source": [
            f"# Load benchmark data\n",
            f"df = pd.read_csv('{csv_path}')\n",
            "print(f'Loaded {len(df)} benchmark results')\n",
            "df"
        ]
    })

    # Summary statistics cell
    cells.append({
        "cell_type": "markdown",
        "metadata": {},
        "source": ["## Summary Statistics"]
    })

    cells.append({
        "cell_type": "code",
        "metadata": {},
        "execution_count": None,
        "outputs": [],
        "source": [
            "# Summary statistics\n",
            "summary = df[['name', 'mean_ns', 'median_ns', 'stddev_ns', 'min_ns', 'max_ns']].copy()\n",
            "summary['cv_%'] = (summary['stddev_ns'] / summary['mean_ns'] * 100).round(2)\n",
            "summary"
        ]
    })

    # Bar chart cell
    cells.append({
        "cell_type": "markdown",
        "metadata": {},
        "source": ["## Mean Execution Time"]
    })

    cells.append({
        "cell_type": "code",
        "metadata": {},
        "execution_count": None,
        "outputs": [],
        "source": [
            "fig, ax = plt.subplots(figsize=(12, 6))\n",
            "bars = ax.bar(df['name'], df['mean_ns'], yerr=df['stddev_ns'], capsize=5)\n",
            "ax.set_xlabel('Benchmark')\n",
            "ax.set_ylabel('Time (ns)')\n",
            "ax.set_title('Mean Execution Time with Standard Deviation')\n",
            "plt.xticks(rotation=45, ha='right')\n",
            "plt.tight_layout()\n",
            "plt.show()"
        ]
    })

    # Percentiles chart cell
    cells.append({
        "cell_type": "markdown",
        "metadata": {},
        "source": ["## Percentile Analysis"]
    })

    cells.append({
        "cell_type": "code",
        "metadata": {},
        "execution_count": None,
        "outputs": [],
        "source": [
            "fig, ax = plt.subplots(figsize=(12, 6))\n",
            "x = range(len(df))\n",
            "width = 0.2\n",
            "\n",
            "ax.bar([i - width for i in x], df['median_ns'], width, label='Median')\n",
            "ax.bar(x, df['p95_ns'], width, label='P95')\n",
            "ax.bar([i + width for i in x], df['p99_ns'], width, label='P99')\n",
            "\n",
            "ax.set_xlabel('Benchmark')\n",
            "ax.set_ylabel('Time (ns)')\n",
            "ax.set_title('Percentile Comparison')\n",
            "ax.set_xticks(x)\n",
            "ax.set_xticklabels(df['name'], rotation=45, ha='right')\n",
            "ax.legend()\n",
            "plt.tight_layout()\n",
            "plt.show()"
        ]
    })

    # Comparison table cell
    cells.append({
        "cell_type": "markdown",
        "metadata": {},
        "source": ["## Relative Performance"]
    })

    cells.append({
        "cell_type": "code",
        "metadata": {},
        "execution_count": None,
        "outputs": [],
        "source": [
            "# Relative performance (compared to fastest)\n",
            "baseline = df['mean_ns'].min()\n",
            "df['relative'] = (df['mean_ns'] / baseline).round(2)\n",
            "df[['name', 'mean_ns', 'relative']].sort_values('mean_ns')"
        ]
    })

    # Custom analysis section
    cells.append({
        "cell_type": "markdown",
        "metadata": {},
        "source": ["## Custom Analysis\n", "\n", "Add your own analysis below:"]
    })

    cells.append({
        "cell_type": "code",
        "metadata": {},
        "execution_count": None,
        "outputs": [],
        "source": ["# Your custom analysis here\n"]
    })

    return cells


def create_notebook(csv_path: str) -> dict:
    """Create a complete Jupyter notebook structure."""
    return {
        "nbformat": 4,
        "nbformat_minor": 4,
        "metadata": {
            "kernelspec": {
                "display_name": "Python 3",
                "language": "python",
                "name": "python3"
            },
            "language_info": {
                "name": "python",
                "version": "3.10.0"
            }
        },
        "cells": create_notebook_cells(csv_path)
    }


def main():
    parser = argparse.ArgumentParser(description="Generate notebook for benchmark analysis")
    parser.add_argument("csv_file", help="Path to benchmark results CSV")
    parser.add_argument("-o", "--output", help="Output notebook path", default="benchmark_analysis.ipynb")
    parser.add_argument("--no-venv", action="store_true", help="Skip venv creation")
    args = parser.parse_args()

    csv_path = Path(args.csv_file)
    output_path = Path(args.output)

    if not args.no_venv:
        ensure_venv(output_path.parent.resolve())

    with open(output_path, 'w') as f:
        json.dump(create_notebook(str(csv_path)), f, indent=2)

    print(f"Generated: {output_path}")


if __name__ == "__main__":
    main()

