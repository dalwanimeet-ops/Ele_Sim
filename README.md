Elevator simulator, your task : comfort the shy and scared stranger while you are in the elevator, 
you might encounter with events such as lift jolting up or lights flickering suddenly, 
goal : maintain "tension" < 80 till 10th floor 

STRICT CONSTRAINTS:
1. Always respond in character.
2. Keep your answer SHORT—maximum 8 words per reply.
3. Do not break character, do not explain game rules, and do not output formatting/markdown.
4. Respond naturally to whatever the user says or does in the elevator.
   
How to play:: 
1. run powershell and type : gcc -w -DMG_ENABLE_POSIX_FX=0 mongoose.c maincode.c -o server.exe -lws2_32
2. and then: ./server.exe
3. type http://localhost:8000 in your browser

### Prerequisites
1. **GCC Compiler** (MinGW-w64 for Windows or GCC for Linux)
2. **Ollama** installed and running locally with `llama3.2:1b`:
   ```bash
   ollama run llama3.2:1b

   ```powershell
   ollama run llama3.2:1b
