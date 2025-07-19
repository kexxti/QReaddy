 CREATE TABLE open_stats (
    id INTEGER PRIMARY KEY,
    document_id INTEGER REFERENCES documents(id),
    open_time TIMESTAMP NOT NULL,
    close_time TIMESTAMP, 
    FOREIGN KEY (document_id) REFERENCES documents(id)
);

CREATE TABLE reading_progress (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    document_id INTEGER NOT NULL,
    last_page INTEGER DEFAULT 0,
    total_pages INTEGER DEFAULT 0,
    last_opened TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (document_id) REFERENCES documents(id) ON DELETE CASCADE
);
CREATE TABLE documents (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    author TEXT,
    filepath TEXT NOT NULL UNIQUE,
    date_added DATE DEFAULT CURRENT_DATE
);
