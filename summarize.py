from transformers import pipeline
import sys

def summarize_text(text):
    try:
        summarizer = pipeline("summarization", model="google/pegasus-xsum")
        if len(text) > 1000:
            text = text[:1000]
        summary = summarizer(text, max_length=150, min_length=30, do_sample=False)
        return summary[0]["summary_text"]
    except Exception as e:
        return f"Ошибка суммаризации: {str(e)}"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Ошибка: текст для суммаризации не передан")
        sys.exit(1)
    input_text = sys.argv[1]
    print(summarize_text(input_text))
