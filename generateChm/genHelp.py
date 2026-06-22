import sys
from pathlib import Path
from helpers import run_pandoc, fix_internal_links_robust, compile_chm
from project_gen import generate_hhp, generate_hhc

def main():
    # Use absolute paths to avoid ambiguity
    doc_dir = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    md_files = list(doc_dir.glob("*.md"))
    
    print("Converting Markdown files...")
    all_html = []
    for md in md_files:
        all_html.append(run_pandoc(md))
        
    print("Fixing internal links...")
    fix_internal_links_robust(all_html, doc_dir)
    
    # Generate project files using the absolute list
    print("Creating CHM project structure...")
    generate_hhp("help.hhp", all_html)
    generate_hhc("help.hhc", all_html)
    
    print("Compiling CHM...")
    compile_chm("help.hhp")
    print("Success: help.chm is ready.")

if __name__ == "__main__":
    main()