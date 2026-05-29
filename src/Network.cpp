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

    Label(w, 5,5, "IP ADDRESS: " + std::string(IP_ADDRESS), CP_LABEL, A_BOLD).draw();
    Label(w, 7,5, "SERVER PORT: " + std::to_string(SERVER_PORT), CP_LABEL, A_BOLD).draw();

    Label(w, 9, 5, "SERVER STATUS: ", CP_LABEL, A_BOLD).draw();
    if(SiriusSystem::getInstance().getServerStatus() == SERVER_LISTEN){
        Label(w, 9, 20, "CONNECTED", CP_GRAPH, A_BLINK).draw();
    }else{
        Label(w, 9, 20, "DISCONNECTED", CP_FAIL, A_BLINK).draw();
    }
    wnoutrefresh(w);
}

NetworkPage::~NetworkPage()
{
}
