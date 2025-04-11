import os
import openai
import torch
import torch.nn.functional as F
import pdfplumber
from transformers import AutoTokenizer, AutoModel

# --- Config ---
DOC_FOLDER = r"D:\Genta\Kolosal-server-play\server-test\python\RAG_test\documents"
EMBED_MODEL = 'intfloat/multilingual-e5-large-instruct'
BASE_URL = "http://localhost:8080/v1"
API_KEY = "sk-dummy"
CHUNK_SIZE = 300  # words per chunk

# --- Embedding Setup ---
tokenizer = AutoTokenizer.from_pretrained(EMBED_MODEL)
model = AutoModel.from_pretrained(EMBED_MODEL)

def average_pool(last_hidden_states, attention_mask):
    last_hidden = last_hidden_states.masked_fill(~attention_mask[..., None].bool(), 0.0)
    return last_hidden.sum(dim=1) / attention_mask.sum(dim=1)[..., None]

def embed_texts(texts):
    inputs = tokenizer(texts, max_length=512, padding=True, truncation=True, return_tensors='pt')
    outputs = model(**inputs)
    embeddings = average_pool(outputs.last_hidden_state, inputs['attention_mask'])
    return F.normalize(embeddings, p=2, dim=1)

# --- PDF Loader and Chunker ---
def chunk_text(text, chunk_size=CHUNK_SIZE):
    words = text.split()
    chunks = [' '.join(words[i:i + chunk_size]) for i in range(0, len(words), chunk_size)]
    return chunks

def extract_chunks_from_pdfs(folder):
    all_chunks = []
    for fname in os.listdir(folder):
        if fname.lower().endswith(".pdf"):
            path = os.path.join(folder, fname)
            with pdfplumber.open(path) as pdf:
                text = "\n".join(page.extract_text() or "" for page in pdf.pages)
                chunks = chunk_text(text)
                for i, chunk in enumerate(chunks):
                    all_chunks.append((f"{fname} [chunk {i}]", chunk))
    return all_chunks

# --- Query ---
def get_instruction(query):
    return f"Instruct: Given a user question, retrieve relevant context to answer it.\nQuery: {query}"

def search(query, chunks):
    query_embedding = embed_texts([get_instruction(query)])
    chunk_texts = [chunk[1] for chunk in chunks]
    chunk_embeddings = embed_texts(chunk_texts)
    scores = (query_embedding @ chunk_embeddings.T) * 100
    ranked = sorted(zip(scores[0].tolist(), chunks), reverse=True)
    return ranked[:5]

# --- Streaming Response ---
def ask_openai(context, query):
    client = openai.OpenAI(base_url=BASE_URL, api_key=API_KEY)
    system = "You are a helpful assistant. Use the provided context to answer the question."
    user_msg = f"Context:\n{context}\n\nQuestion: {query}"
    
    stream = client.chat.completions.create(
        model="Qwen2.5 0.5B",
        messages=[{"role": "system", "content": system},
                  {"role": "user", "content": user_msg}],
        stream=True
    )
    
    print("\nAnswer:")
    for chunk in stream:
        if chunk.choices[0].delta.content:
            print(chunk.choices[0].delta.content, end="", flush=True)
    print()

# --- Main App ---
if __name__ == "__main__":
    print("🧠 RAG Terminal App\n")
    chunks = extract_chunks_from_pdfs(DOC_FOLDER)
    if not chunks:
        print(f"No PDF files or extractable text found in '{DOC_FOLDER}/'.")
        exit()

    while True:
        query = input("\n🔍 Enter your question (or type 'exit'): ").strip()
        if query.lower() in ['exit', 'quit']:
            break
        top_chunks = search(query, chunks)
        context = "\n---\n".join([f"[{meta}]\n{text}" for meta, text in top_chunks])
        ask_openai(context, query)
