# QReaddy

QReaddy is a PDF book reader and manager developed during a summer practice project in 2025.  
It enables users to organize a PDF book collection, track reading progress, and export statistics to CSV for visualization.  
Built using Qt 6.9.1 and Poppler, it provides a user-friendly interface for managing and reading PDF books, with Scilab integration for data plotting.

---

## Features

- **Book Management**: Add, organize, and manage PDF books with details such as title, author, and file path.
- **Progress Tracking**: Monitor reading progress for each book, shown as a percentage of pages read.
- **PDF Reader**: View PDF books with basic navigation (⚠️ under development; may experience flickering during navigation).
- **Statistics Export**: Export book metadata and reading progress to CSV files for external analysis and visualization.

---

## Installation

### Prerequisites

- **Qt**: version 6.9.1 (Community Edition, LGPL v3 license)
- **Poppler**: PDF rendering library (GPL v2/v3 license)
- **PostgreSQL**: database for storing book and progress data
- **Scilab** (optional): used for generating plots from CSV files
- A C++ compiler (e.g. GCC or Clang)

---

## Usage

- **Add Books**: Use the "Add Book" button to import PDF files and provide title, author, and file path.
- **Track Progress**: Progress is updated automatically in the integrated PDF viewer and stored in the database.
- **View PDFs**: Open books in the built-in PDF viewer.
- **Export Statistics**: Use the "Export" button to save book and reading data to a CSV file (e.g. `data4.csv`).
- **Generate Plots**: Open the exported CSV in Scilab and run custom scripts to generate visualizations.

---

## Possible Extensions

- **AI Integration**: Add AI-based book summarization using local models or APIs (e.g. xAI, GPT).
- **Matplotlib Support**: Replace Scilab with Python’s Matplotlib for broader visualization options.
- **Bug Fixes**: Improve PDF reader performance and eliminate flickering issues.
- **UI Improvements**: Refine window resizing behavior and optimize thumbnail rendering in `PdfCoverDelegate`.
- **Database Enhancements**: Add full-text search, filters by author/title/progress, and sort options.

---

## License

This project is licensed under the MIT License.

It uses the following dependencies:

- **Qt 6.9.1** (Community Edition) — LGPL v3  
- **Poppler** — GPL v2 or v3  
- **PostgreSQL** — PostgreSQL License  
- **Scilab** — CeCILL License (used optionally)

---

## Acknowledgments

Developed as part of a summer practice project in 2025.  
Special thanks to the Qt, Poppler, PostgreSQL, and Scilab communities for their excellent tools and libraries.
