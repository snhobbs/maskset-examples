import sys
import csv
import click
import pandas as pd


@click.command()
@click.option(
    "--delimiter",
    "-d",
    default=",",
    show_default=True,
    help="CSV delimiter.",
)
@click.option(
    "--sort/--no-sort",
    default=True,
    show_default=True,
    help="Sort by S2 then P2.",
)
@click.argument("source", type=click.File("r"), default=sys.stdin)
@click.argument("out", type=click.File("w"), default=sys.stdout)
def to_markdown(source, out, delimiter, sort):
    """
    Convert a CSV/TSV table into a Markdown table.
    Reads from SOURCE or stdin if not provided.
    Optionally sorts by S1 then P1.
    """
    df = pd.read_csv(source, delimiter=delimiter)
    df.columns = [pt.strip() for pt in df.columns]
    df = df.round(3)

    if not len(df):
        click.echo("No data found.", err=True)
        sys.exit(1)

    if sort:
        try:
            df["S2"].astype(int)
            df["P2"].astype(int)
            df.sort_values(by=["S2", "P2"], inplace=True)

        except (ValueError, IndexError):
            click.echo("Could not sort: missing S1 or P1 column", err=True)

    header = list(df.columns)
    rows = df.astype(str).values.tolist()

    # Compute column widths
    col_widths = [max(len(str(cell)) for cell in col)
                  for col in zip(header, *rows)]

    # Format a single row
    def fmt_row(row):
        return "| " + " | ".join(str(cell).ljust(width) for cell, width in zip(row, col_widths)) + " |"

    # Build Markdown table
    header_line = fmt_row(header)
    separator = "| " + " | ".join("-" * width for width in col_widths) + " |"
    body_lines = "\n".join(fmt_row(row) for row in rows)

    out.write("\n".join([header_line, separator, body_lines]) + "\n")


if __name__ == "__main__":
    to_markdown()
