import os
from pathlib import Path


def merge_arduino_project(project_dir: str = ".", output_filename: str = "merged_project.txt"):
    base_path = Path(project_dir).resolve()
    output_path = base_path / output_filename

    # Zbieramy pliki .ino oraz .h (pomijamy wygenerowany plik wyjściowy)
    valid_extensions = {".ino", ".h"}
    files = [
        f for f in base_path.rglob("*")
        if f.is_file() and f.suffix.lower() in valid_extensions and f.name != output_filename
    ]

    if not files:
        print(f"Nie znaleziono żadnych plików .ino ani .h w katalogu: {base_path}")
        return

    # Logika priorytetu sortowania:
    # 1. Główny plik .ino z wielkiej litery (priorytet 0)
    # 2. Pliki nagłówkowe .h (priorytet 1)
    # 3. Pozostałe pliki .ino (priorytet 2)
    def file_sort_key(f: Path):
        name = f.stem
        ext = f.suffix.lower()

        if ext == ".ino" and name and name[0].isupper():
            priority = 0
        elif ext == ".h":
            priority = 1
        else:
            priority = 2

        return (priority, f.name.lower())

    sorted_files = sorted(files, key=file_sort_key)

    with open(output_path, "w", encoding="utf-8") as out:
        # Spis treści / metadane dla modelu
        out.write("# Scalony projekt Arduino / C++\n")
        out.write(f"# Katalog źródłowy: {base_path.name}\n")
        out.write(f"# Liczba plików: {len(sorted_files)}\n\n")
        out.write("## Zawarte pliki:\n")
        for f in sorted_files:
            rel_path = f.relative_to(base_path)
            out.write(f"- {rel_path}\n")
        out.write("\n" + "=" * 80 + "\n\n")

        # Dołączanie zawartości plików
        for f in sorted_files:
            rel_path = f.relative_to(base_path)
            lang = "cpp" if f.suffix.lower() in [".ino", ".h"] else ""

            out.write(f"// ============================================================================\n")
            out.write(f"// PLIK: {rel_path}\n")
            out.write(f"// ============================================================================\n")
            out.write(f"```{lang}\n")

            try:
                content = f.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                # Awaryjne kodowanie w razie nietypowych znaków
                content = f.read_text(encoding="latin-1")

            out.write(content.rstrip())
            out.write("\n```\n\n")

    print(f"Gotowe! Scalono {len(sorted_files)} plików.")
    print(f"Wynik zapisano w: {output_path}")


if __name__ == "__main__":
    # Jeśli skrypt leży w folderze projektu, zostaw "."
    # Możesz też wpisać pełną ścieżkę, np. r"C:\sciezka\do\projektu"
    KATALOG_PROJEKTU = "."
    
    merge_arduino_project(KATALOG_PROJEKTU)