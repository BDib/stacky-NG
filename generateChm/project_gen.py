from pathlib import Path

def generate_hhp(hhp_file, html_files, title="StackyNG Help"):
    # Ensure html_files are relative to where the hhp file is
    content = [
        "[OPTIONS]", f"Title={title}", "Default topic=Docs/README.html",
        "Compiled file=help.chm", "Contents file=help.hhc",
        "[FILES]"
    ] + [str(f.relative_to(Path.cwd())) for f in html_files]
    
    with open(hhp_file, "w", encoding="utf-8") as f:
        f.write("\n".join(content))

def generate_hhc(hhc_file, html_files):
    # Separate README to ensure it's at the top
    readme = next((f for f in html_files if f.stem.lower() == "readme"), None)
    others = sorted([f for f in html_files if f.stem.lower() != "readme"], key=lambda x: x.name)
    
    sorted_files = ([readme] if readme else []) + others
    
    toc = ['<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">', '<UL>']
    for html in sorted_files:
        rel_path = str(html.relative_to(Path.cwd()))
        title = html.stem.replace('_', ' ').title()
        toc.append(f'  <LI> <OBJECT type="text/sitemap"><param name="Name" value="{title}"><param name="Local" value="{rel_path}"></OBJECT>')
    toc.append('</UL>')
    
    with open(hhc_file, "w", encoding="utf-8") as f:
        f.write("\n".join(toc))

def fix_internal_links(html_files):
    """
    Search all generated HTML files and replace .md links with .html
    """
    for html_file in html_files:
        with open(html_file, "r", encoding="utf-8") as f:
            content = f.read()
        
        # Replace occurrences of .md with .html in hrefs
        new_content = content.replace(".md", ".html")
        
        with open(html_file, "w", encoding="utf-8") as f:
            f.write(new_content)