#include "mongoose.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

typedef struct {
    int tension;
    int comfort;
    int floor;
    int target_floor;
    int is_game_over;
    int is_win;
    char event_text[128];
} SessionState;

static SessionState g_session = {20, 50, 1, 10, 0, 0, ""};

static const char *s_html_ui = 
"<!DOCTYPE html>"
"<html><head><title>Elevator Simulator</title>"
"<style>"
"body{font-family:sans-serif;background:#0f172a;color:#f8fafc;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;}"
".container{width:460px;background:#1e293b;padding:24px;border-radius:12px;box-shadow:0 10px 25px rgba(0,0,0,0.5);border:1px solid #334155;}"
"h2{text-align:center;color:#38bdf8;margin:0;}"
".mood-box{text-align:center;font-size:42px;margin:10px 0 5px 0;}"
".stats{display:flex;justify-content:space-between;background:#0f172a;padding:12px;border-radius:8px;margin:15px 0;}"
".stat-box{text-align:center;}"
".stat-val{font-size:18px;font-weight:bold;color:#f43f5e;}"
".event-banner{background:#7c2d12;color:#fdba74;padding:8px;border-radius:6px;text-align:center;font-size:13px;font-weight:bold;margin-bottom:12px;display:none;}"
".dialogue-box{height:160px;background:#0f172a;border:1px solid #334155;border-radius:8px;padding:12px;overflow-y:auto;margin-bottom:15px;font-style:italic;color:#cbd5e1;}"
".input-group{display:flex;gap:8px;}"
"input{flex:1;padding:10px;border-radius:6px;border:1px solid #334155;background:#0f172a;color:#fff;}"
"button{padding:10px 16px;background:#0284c7;color:white;border:none;border-radius:6px;cursor:pointer;font-weight:bold;}"
"#resetBtn{background:#10b981;margin-top:10px;width:100%;display:none;}"
".game-over{text-align:center;font-weight:bold;margin-top:10px;}"
"</style></head><body>"
"<div class='container'>"
"<h2>Elevator Simulator</h2>"
"<div id='mood' class='mood-box'>😐</div>"
"<div class='stats'>"
"<div class='stat-box'>Tension<div id='tension' class='stat-val'>20%</div></div>"
"<div class='stat-box'>Comfort<div id='comfort' class='stat-val' style='color:#10b981;'>50%</div></div>"
"<div class='stat-box'>Floor<div id='floor' class='stat-val' style='color:#fbbf24;'>1 / 10</div></div>"
"</div>"
"<div id='eventBanner' class='event-banner'></div>"
"<div id='dialogue' class='dialogue-box'><strong>Stranger:</strong> ... (Stares awkwardly at floor buttons)</div>"
"<div class='input-group'>"
"<input type='text' id='userInput' placeholder='Talk to the stranger...' onkeydown=\"if(event.key==='Enter') sendAction()\">"
"<button id='sendBtn' onclick='sendAction()'>Send</button>"
"</div>"
"<button id='resetBtn' onclick='resetGame()'>Try Again</button>"
"<div id='status' class='game-over'></div>"
"</div>"
"<script>"
"async function sendAction(){"
"  const input = document.getElementById('userInput');"
"  const text = input.value.trim();"
"  if(!text) return;"
"  const dialogueBox = document.getElementById('dialogue');"
"  dialogueBox.innerHTML += `<br><strong>You:</strong> ${text}`;"
"  input.value = '';"
"  try {"
"    const res = await fetch('/api/action', {"
"      method: 'POST',"
"      headers: {'Content-Type': 'application/x-www-form-urlencoded'},"
"      body: `choice=${encodeURIComponent(text)}`"
"    });"
"    const data = await res.json();"
"    document.getElementById('tension').innerText = data.tension + '%';"
"    document.getElementById('comfort').innerText = data.comfort + '%';"
"    document.getElementById('floor').innerText = data.floor + ' / ' + data.target_floor;"
"    document.getElementById('mood').innerText = data.mood;"
"    "
"    const eb = document.getElementById('eventBanner');"
"    if(data.event_text && data.event_text.length > 0){"
"      eb.innerText = '⚡ EVENT: ' + data.event_text;"
"      eb.style.display = 'block';"
"    } else { eb.style.display = 'none'; }"
"    "
"    dialogueBox.innerHTML += `<br><strong>Stranger:</strong> ${data.reply}`;"
"    dialogueBox.scrollTop = dialogueBox.scrollHeight;"
"    if(data.game_over){"
"      const statusDiv = document.getElementById('status');"
"      if(data.is_win){"
"        statusDiv.style.color = '#10b981';"
"        statusDiv.innerText = '🎉 YOU WIN! Reached Floor 10 safely without scaring the stranger!';"
"      } else {"
"        statusDiv.style.color = '#ef4444';"
"        statusDiv.innerText = '💥 GAME OVER: Stranger hit emergency stop button and fled!';"
"      }"
"      input.disabled = true;"
"      document.getElementById('sendBtn').disabled = true;"
"      document.getElementById('resetBtn').style.display = 'block';"
"    }"
"  } catch(e) { console.error(e); }"
"}"
"async function resetGame(){"
"  await fetch('/api/reset', {method: 'POST'});"
"  location.reload();"
"}"
"</script></body></html>";

void call_ollama(const char *user_input, char *ai_reply, size_t reply_size) {
    FILE *req = fopen("req.json", "w");
    if (req != NULL) {
        fprintf(req, "{\n");
        fprintf(req, "  \"model\": \"llama3.2:1b\",\n");
        fprintf(req, "  \"prompt\": \"You are a passenger in an elevator. Respond awkwardly in max 8 words. User says: ");
        
        for (const char *p = user_input; *p != '\0'; p++) {
            if (*p == '"' || *p == '\\' || *p == '\n' || *p == '\r') fprintf(req, " ");
            else fputc(*p, req);
        }
        
        fprintf(req, "\",\n");
        fprintf(req, "  \"stream\": false\n");
        fprintf(req, "}\n");
        fclose(req);
    }

    FILE *fp = _popen("curl.exe -s -m 10 -H \"Content-Type: application/json\" -d @req.json http://localhost:11434/api/generate", "r");
    if (fp == NULL) {
        snprintf(ai_reply, reply_size, "...looks at the elevator buttons.");
        return;
    }

    char response[4096] = {0};
    size_t n = fread(response, 1, sizeof(response) - 1, fp);
    _pclose(fp);

    if (n == 0) {
        snprintf(ai_reply, reply_size, "...nods quietly.");
        return;
    }

    char *parsed = strstr(response, "\"response\":\"");
    if (parsed) {
        parsed += 12;
        size_t i = 0;
        while (*parsed != '"' && *parsed != '\0' && i < reply_size - 1) {
            if (*parsed == '\\' && *(parsed + 1) == '"') {
                parsed += 2;
                continue;
            }
            if (*parsed == '\\' && *(parsed + 1) == 'n') {
                parsed += 2;
                ai_reply[i++] = ' ';
                continue;
            }
            if (*parsed == '\n' || *parsed == '\r' || *parsed == '"') {
                parsed++;
                continue;
            }
            ai_reply[i++] = *parsed++;
        }
        ai_reply[i] = '\0';
    } else {
        snprintf(ai_reply, reply_size, "...uh, okay.");
    }

    if (strlen(ai_reply) == 0) {
        snprintf(ai_reply, reply_size, "...stares at the floor display.");
    }
}

void process_user_action(const char *user_text, char *reply_buf, size_t buf_size) {
    if (g_session.is_game_over) return;

    g_session.event_text[0] = '\0';

    // 1. Lowercase sentiment parsing
    char lower_text[256] = {0};
    for (int i = 0; user_text[i] && i < 255; i++) {
        lower_text[i] = tolower((unsigned char)user_text[i]);
    }

    if (strstr(lower_text, "hi") || strstr(lower_text, "hello") || strstr(lower_text, "sorry") || 
        strstr(lower_text, "cool") || strstr(lower_text, "nice") || strstr(lower_text, "fine") ||
        strstr(lower_text, "relax") || strstr(lower_text, "ok") || strstr(lower_text, "floor")) {
        
        g_session.tension -= 10;
        g_session.comfort += 10;
    } 
    else if (strstr(lower_text, "stare") || strstr(lower_text, "touch") || strstr(lower_text, "scared") ||
             strstr(lower_text, "why") || strstr(lower_text, "kill") || strstr(lower_text, "weird") ||
             strstr(lower_text, "follow")) {
        
        g_session.tension += 25;
        g_session.comfort -= 15;
    } 
    else {
        g_session.tension += 5;
    }

    // 2. Random Elevator Events (30% chance each floor)
    int rand_val = rand() % 100;
    if (rand_val < 25) {
        g_session.tension += 15;
        snprintf(g_session.event_text, sizeof(g_session.event_text), "Elevator lights flickered violently!");
    } else if (rand_val > 80) {
        g_session.tension += 10;
        snprintf(g_session.event_text, sizeof(g_session.event_text), "Elevator suddenly jolted while going up!");
    }

    // 3. Clamping & Progress
    if (g_session.tension < 0) g_session.tension = 0;
    if (g_session.tension > 100) g_session.tension = 100;
    if (g_session.comfort < 0) g_session.comfort = 0;
    if (g_session.comfort > 100) g_session.comfort = 100;

    g_session.floor++;

    call_ollama(user_text, reply_buf, buf_size);

    // Win/Loss Rules
    if (g_session.floor >= g_session.target_floor && g_session.tension < 80) {
        g_session.is_game_over = 1;
        g_session.is_win = 1;
    } else if (g_session.tension >= 100 || g_session.floor >= g_session.target_floor) {
        g_session.is_game_over = 1;
        g_session.is_win = 0;
    }
}

const char* get_mood_emoji(int tension, int comfort) {
    if (tension >= 75) return "😱";
    if (tension >= 50) return "😬";
    if (comfort >= 65) return "🙂";
    return "😐";
}

static void handle_api_action(struct mg_connection *c, struct mg_http_message *hm) {
    char user_input[256] = {0};
    char npc_reply[256] = {0};

    mg_http_get_var(&hm->body, "choice", user_input, sizeof(user_input));
    process_user_action(user_input, npc_reply, sizeof(npc_reply));

    for (int k = 0; npc_reply[k] != '\0'; k++) {
        if (npc_reply[k] == '"' || npc_reply[k] == '\\' || npc_reply[k] == '\n' || npc_reply[k] == '\r') {
            npc_reply[k] = '\'';
        }
    }

    char json_buf[1024];
    snprintf(json_buf, sizeof(json_buf),
        "{"
            "\"tension\":%d,"
            "\"comfort\":%d,"
            "\"floor\":%d,"
            "\"target_floor\":%d,"
            "\"mood\":\"%s\","
            "\"event_text\":\"%s\","
            "\"reply\":\"%s\","
            "\"game_over\":%s,"
            "\"is_win\":%s"
        "}",
        g_session.tension,
        g_session.comfort,
        g_session.floor,
        g_session.target_floor,
        get_mood_emoji(g_session.tension, g_session.comfort),
        g_session.event_text,
        npc_reply,
        g_session.is_game_over ? "true" : "false",
        g_session.is_win ? "true" : "false"
    );

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json_buf);
}

static void handle_api_reset(struct mg_connection *c) {
    g_session.tension = 20;
    g_session.comfort = 50;
    g_session.floor = 1;
    g_session.target_floor = 10;
    g_session.is_game_over = 0;
    g_session.is_win = 0;
    g_session.event_text[0] = '\0';
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"ok\"}");
}

static void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        if (hm->uri.len >= 11 && strncmp(hm->uri.buf, "/api/action", 11) == 0) {
            handle_api_action(c, hm);
        } else if (hm->uri.len >= 10 && strncmp(hm->uri.buf, "/api/reset", 10) == 0) {
            handle_api_reset(c);
        } else {
            mg_http_reply(c, 200, "Content-Type: text/html\r\n", "%s", s_html_ui);
        }
    }
}

int main(void) {
    srand((unsigned int)time(NULL));
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    printf("==================================================\n");
    printf(" Web UI Active on http://localhost:8000\n");
    printf("==================================================\n");

    mg_http_listen(&mgr, "http://localhost:8000", event_handler, NULL);

    for (;;) {
        mg_mgr_poll(&mgr, 1000);
    }

    mg_mgr_free(&mgr);
    return 0;
}