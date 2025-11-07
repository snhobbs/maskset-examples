# Local LLMs

- Use an LLM without a network connection
- Use a chatbot without leaking information
- Build highly specific agents
- See your laptop smoke

## Ollama

- [Repo](https://github.com/ollama/ollama)
- [List of Models](ollama.com/search)

### Start Container & Server

```sh
docker run -d -v ollama:/root/.ollama -p 11434:11434 --name ollama ollama/ollama serve
```

Port 11434 is the ollama standard.

### Test Server

```sh
curl http://localhost:11434/api/generate -d '{
  "model": "gemma3:1b",
  "prompt":"Why is the sky blue?"
}'
```

Response:

```sh
{"model":"gemma3:1b","created_at":"2025-11-03T16:19:19.190831559Z","response":"Okay","done":false}
...
```

For a command line chatbot you can run:

```sh
ollama run <MODEL NAME>
```

```
>>> Send a message (/? for help)
```

### Pull a Model

```
ollama pull <MODEL>
```

Use `docker exec` to run in a running container

```sh
docker exec -it ollama ollama pull deepseek-r1
```

## OpenWebUI

```sh
docker run -d -p 3000:8080 --add-host=host.docker.internal:host-gateway -v open-webui:/app/backend/data --name open-webui --restart always ghcr.io/open-webui/open-webui:main
```

The server can be accessed with http at `0.0.0.0:3000`

## RAG/Knowledge Base Options

- <https://casibase.org/docs/overview/>
- <https://github.com/SciPhi-AI/R2R>
- <https://github.com/nickthecook/archyve>
- <https://github.com/danny-avila/LibreChat>
