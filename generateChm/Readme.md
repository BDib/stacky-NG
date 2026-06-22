# Help System Documentation (`genHelp.py`)

## Overview

`genHelp.py` is the orchestration engine used to compile your Markdown documentation into a **Windows Help (`.chm`)** file. It handles Markdown-to-HTML conversion, TOC generation, internal link resolution, and final compilation.

## ⚠️ Important Entry Point Requirement

The system is hard-coded to recognize **`README.md`** as the project entry point.

## Prerequisites

To compile the help system, the following tools must be installed and available in your System PATH:

1. **Pandoc**: Used to convert Markdown to HTML.
* *Install*: `winget install pandoc.pandoc` or `choco install pandoc`


2. **HTML Help Workshop**: The official Microsoft tool to compile CHM files.
* *Download*: [Microsoft HTML Help Workshop](https://www.google.com/search?q=https://learn.microsoft.com/en-us/previous-versions/windows/desktop/aa962534(v%3Dvs.85))
* Ensure `hhc.exe` is installed, typically in `C:\Program Files (x86)\HTML Help Workshop\`.


3. **Python 3.x**: With `BeautifulSoup4` installed.
* *Install*: `pip install beautifulsoup4`



## Usage

Run the script from the command line, pointing it to the directory containing your Markdown files:

```bash
python genHelp.py <path_to_markdown_directory>

```

### Options

* `--title "Your Title"`: Sets the display title for the CHM window.
* `--no-compile`: Stops after generating the project files (`.hhp`, `.hhc`). Useful for debugging the structure without triggering the full compile.
* `-h, --help`: Displays usage information.

## Workflow Details

1. **Conversion**: The script iterates through all `.md` files in the target directory and converts them to `.html`.
2. **Link Normalization**: It performs a case-insensitive regex search-and-replace to ensure all internal links (e.g., `[Tutorial](TUTORIAL.md)`) are converted to `Tutorial.html`, ensuring they work inside the CHM viewer.
3. **Project Structuring**: It generates the required `.hhp` (Project) and `.hhc` (Table of Contents) files.
4. **Sorting**: It enforces a custom sort order, ensuring `README.md` is always the first item in the navigation tree.
5. **Compilation**: Finally, it calls `hhc.exe` to package everything into `help.chm`.

---

### Tips for Maintenance

* **Internal Linking**: Always use standard Markdown links (`[Link Text](filename.md)`). The script will automatically convert these to `.html` references during the build.
* **Troubleshooting**: If links break, check the `help.hhc` file. It lists the `Local` path for every item; if a file is missing or misspelled, the CHM viewer will throw an error upon opening.
* **Styling**: Add a `style.css` file to your documentation directory to define the visual appearance of your HTML pages. The script automatically injects this CSS during the Pandoc conversion step.