#include "Network.h"
#include "config.h"
#include "system.h"

NetworkPage::NetworkPage(): Screen("Network")
{
}

void NetworkPage::draw(Panel *p, bool focused)
{
    WINDOW* w = p->inner();
    werase(w);

    auto& gs = SiriusSystem::getInstance().getGroundStation();

    // 1. Title and Config parameters
    wattron(w, COLOR_PAIR(CP_LABEL) | A_BOLD);
    mvwprintw(w, 2, 4, "=== NETWORK CONFIGURATION ===");
    wattroff(w, COLOR_PAIR(CP_LABEL) | A_BOLD);
    
    Label(w, 4, 6, "Remote Server IP:   " + std::string(IP_ADDRESS), CP_NORMAL).draw();
    Label(w, 5, 6, "Local Server Port:  " + std::to_string(SERVER_PORT), CP_NORMAL).draw();
    Label(w, 6, 6, "Local Client Port:  " + std::to_string(CLIENT_PORT), CP_NORMAL).draw();
    Label(w, 7, 6, "Remote Target Port: " + std::to_string(REMOTE_SERVER_PORT), CP_NORMAL).draw();

    // 2. Interface and Socket Status
    int y = 9;
    wattron(w, COLOR_PAIR(CP_LABEL) | A_BOLD);
    mvwprintw(w, y++, 4, "=== INTERFACE & SOCKET STATUS ===");
    wattroff(w, COLOR_PAIR(CP_LABEL) | A_BOLD);

    Label(w, y, 6, "Local Server Socket (Port " + std::to_string(SERVER_PORT) + "):", CP_NORMAL).draw();
    {
        auto srv_status = SiriusSystem::getInstance().getServerStatus();
        if (srv_status == SERVER_CONNECTED) {
            Label(w, y, 46, "[ CONNECTED ]", CP_GRAPH, A_BOLD).draw();
            y++;
            Label(w, y, 8, "Connected Client: " + gs.getServer().getConnectedClientIP() + ":" + std::to_string(gs.getServer().getConnectedClientPort()), CP_LABEL).draw();
        } else if (srv_status == SERVER_LISTEN) {
            Label(w, y, 46, "[ LISTENING ]", CP_GRAPH, A_BOLD).draw();
        } else {
            Label(w, y, 46, "[ ERROR / CLOSED ]", CP_FAIL, A_BOLD).draw();
        }
    }
    y++;

    Label(w, y, 6, "Local Client Socket (Port " + std::to_string(CLIENT_PORT) + "):", CP_NORMAL).draw();
    if (gs.getClient().getStatus() == CLIENT_READY) {
        Label(w, y, 46, "[ READY ]", CP_GRAPH, A_BOLD).draw();
    } else {
        Label(w, y, 46, "[ ERROR / OFFLINE ]", CP_FAIL, A_BOLD).draw();
    }
    y += 2;

    // 3. Client & Server Connection Status (Heartbeats)
    wattron(w, COLOR_PAIR(CP_LABEL) | A_BOLD);
    mvwprintw(w, y++, 4, "=== CONNECTION STATUS (HEARTBEATS) ===");
    wattroff(w, COLOR_PAIR(CP_LABEL) | A_BOLD);

    Label(w, y, 6, "Remote Dashboard Link: ", CP_NORMAL).draw();
    if (gs.isServerDashboardConnected()) {
        Label(w, y, 32, "CONNECTED", CP_GRAPH, A_BOLD).draw();
    } else {
        Label(w, y, 32, "DISCONNECTED", CP_FAIL, A_BOLD).draw();
    }
    y++;

    Label(w, y, 6, "Engine Board Client:   ", CP_NORMAL).draw();
    if (gs.isEngineConnected()) {
        Label(w, y, 32, "CONNECTED", CP_GRAPH, A_BOLD).draw();
    } else {
        Label(w, y, 32, "DISCONNECTED", CP_FAIL, A_BOLD).draw();
    }
    y++;

    Label(w, y, 6, "Fill Station Client:   ", CP_NORMAL).draw();
    if (gs.isFillConnected()) {
        Label(w, y, 32, "CONNECTED", CP_GRAPH, A_BOLD).draw();
    } else {
        Label(w, y, 32, "DISCONNECTED", CP_FAIL, A_BOLD).draw();
    }
    y += 2;

    // 4. RX/TX Telemetry Table
    wattron(w, COLOR_PAIR(CP_LABEL) | A_BOLD);
    mvwprintw(w, y++, 4, "=== UDP LINK STATISTICS ===");
    wattroff(w, COLOR_PAIR(CP_LABEL) | A_BOLD);

    // Table Header
    wattron(w, COLOR_PAIR(CP_NORMAL) | A_UNDERLINE);
    mvwprintw(w, y++, 6, "%-35s %-16s %-16s %-12s", "Link Name", "Packets (RX/TX)", "Bytes (RX/TX)", "CRC Errors");
    wattroff(w, COLOR_PAIR(CP_NORMAL) | A_UNDERLINE);

    // Server Link metrics (Local Client socket connected to remote server)
    mvwprintw(w, y++, 6, "%-35s %-7u / %-7u %-7lu / %-7lu %-12u",
              "Server Link (to remote Dashboard)",
              gs.getClientRxPackets(), gs.getClientTxPackets(),
              gs.getClientRxBytes(), gs.getClientTxBytes(),
              gs.getClientCrcErrors());

    // Client Link metrics (Local Server socket listening for client boards)
    mvwprintw(w, y++, 6, "%-35s %-7u / %-7u %-7lu / %-7lu %-12u",
              "Client Link (from Boards)",
              gs.getServerRxPackets(), gs.getServerTxPackets(),
              gs.getServerRxBytes(), gs.getServerTxBytes(),
              gs.getServerCrcErrors());

    wnoutrefresh(w);
}

NetworkPage::~NetworkPage()
{
}
