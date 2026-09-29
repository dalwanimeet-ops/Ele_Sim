Elevator simulator, your task : comfort the shy and scared stranger while you are in the elevator, 
you might encounter with events such as lift jolting up or lights flickering suddenly, 
goal : maintain "tension" < 80 till 10th floor 

How to play:: 
1. run powershell and type : gcc -w -DMG_ENABLE_POSIX_FX=0 mongoose.c maincode.c -o server.exe -lws2_32
2. and then: ./server.exe
3. type http://localhost:8000 in your browser
