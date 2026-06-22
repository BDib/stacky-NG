import subprocess, shutil, os, re
from pathlib import Path
from bs4 import BeautifulSoup

def run_pandoc(md_path):
    """Converts MD to HTML using Pandoc."""
    html_path = md_path.with_suffix(".html")
    # --standalone creates a full HTML doc
    cmd = ["pandoc", str(md_path), "-o", str(html_path), "--standalone", "--css=style.css"]
    subprocess.run(cmd, check=True)
    return html_path

def get_linked_files(html_path, base_dir):
    """Parses HTML to find internal links to other generated docs."""
    linked = set()
    with open(html_path, "r", encoding="utf-8") as f:
        soup = BeautifulSoup(f, "html.parser")
        for a in soup.find_all("a", href=True):
            href = a["href"]
            # Look for links that point to other .md files
            if href.endswith(".md"):
                target = (base_dir / href).with_suffix(".html")
                if target.exists():
                    linked.add(target)
    return linked

def fix_internal_links_robust(html_files, doc_dir):
    # This regex looks for href="... .md" regardless of case
    md_pattern = re.compile(r'href=["\']([^"\']+\.md)["\']', re.IGNORECASE)
    
    # Mapping of target filenames to their corrected .html versions
    name_map = {
        "setup_stacky": "SetupStacky",
        "setupstacky": "SetupStacky",
        "build": "BUILD"
    }

    for html_file in html_files:
        with open(html_file, "r", encoding="utf-8") as f:
            content = f.read()

        def replace_link(match):
            original_link = match.group(1) # e.g., "BUILD.md"
            base_name = Path(original_link).stem.lower() # e.g., "build"
            
            # Use mapped name if exists, else keep stem
            new_base = name_map.get(base_name, Path(original_link).stem)
            return f'href="{new_base}.html"'

        # Perform the regex replacement
        new_content = md_pattern.sub(replace_link, content)
        
        with open(html_file, "w", encoding="utf-8") as f:
            f.write(new_content)
            
def compile_chm(hhp_file):
    hhc_path = r"C:\Program Files (x86)\HTML Help Workshop\hhc.exe"
    if not os.path.exists(hhc_path):
        raise FileNotFoundError("HTML Help Workshop not found at " + hhc_path)
    subprocess.run([hhc_path, str(hhp_file)])