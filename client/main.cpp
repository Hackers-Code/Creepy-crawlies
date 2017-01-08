#include <iostream>
#include <math.h>
#include <cstdlib>
#include <windows.h>
#include <iomanip>
#include <list>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Audio.hpp>

#define INFO_AMOUNT 12

using namespace std;

bool copyToClipboard(string input){
    HGLOBAL hglbCopy;
    LPTSTR  lptstrCopy;

    if(!OpenClipboard(0))
        return 0;
    EmptyClipboard();
    if(!input.length()){
        CloseClipboard();
        return 0;
    }

    hglbCopy=GlobalAlloc(GMEM_MOVEABLE, input.length()+1);
    if(!hglbCopy){
        CloseClipboard();
        return 0;
    }

    memcpy(GlobalLock(hglbCopy), &input[0], input.length()+1);
    GlobalUnlock(hglbCopy);
    SetClipboardData(CF_TEXT, hglbCopy);
    CloseClipboard();
    return 1;
}

sf::TcpSocket clientsocket;
sf::UdpSocket udpsocket;
sf::IpAddress serverip;
sf::Socket::Status status;
unsigned char data[1024]={1}, receiving=0;
unsigned int deltareceive=0, udpdeltareceive=0;
unsigned short udpport;
sf::Texture ont, offt;
sf::Sprite connectionS;
bool connected=0;

bool connect(string ip, string port){
    sf::IpAddress ipaddress0(ip);
    if(ipaddress0==sf::IpAddress::None){
        cout<<"invalid ip\n";
        return 0;
    }
    udpport=atoi(port.c_str());
    status=clientsocket.connect(ipaddress0, udpport);
    if(status!=sf::Socket::Done)
    {
        cout<<"not connected\n";
        return 0;
    }
    clientsocket.setBlocking(0);
    cout<<clientsocket.getRemotePort()<<"\n";
    udpsocket.setBlocking(0);
    if(udpsocket.bind(udpsocket.getLocalPort())!=sf::Socket::Done){
        cout<<"udp socket can not be bound to "<<port<<"\n";
    }else{
        udpport=udpsocket.getLocalPort();
        cout<<"udp socket bound to "<<udpport<<"\n";
    }
    connectionS.setTexture(ont);
    connected=1;
    serverip=ip;
/*
    unsigned char to_send[3];
    to_send[0]=0x34;
    to_send[1]=(udpsocket.getLocalPort()>>8)%256;
    to_send[2]=udpsocket.getLocalPort()%256;
    if(clientsocket.send(to_send, 3)==sf::Socket::Done){
    }else cout<<"sending error 0x34\n";
*/
    return 1;
}

//{
sf::RenderWindow window(sf::VideoMode(1200, 720), "worms");
sf::Event event;
sf::Color bgcolor(40,40,40), checkedclr(0,255,255), normalclr(0,255,0), yourcolor(0,0,0);
sf::Texture inputbart, binputbart, okt, bgplanet, reloadgamelistt, soundicont, soundbart, soundpointert, colorpointert, colorbart[3], maskt[10], maskchooset[10], maskrt, masklt;
sf::Sprite  inputbars, binputbars, okconnects, oknicks, okcreaterooms, okrgbm, bgplanes, reloadgamelists, soundicons, soundbars, soundpointers, colorpointers[3], colorbars[3], maskchoose, maskrs, maskls;
sf::Image bgplanei;
sf::Font mainfont;
sf::Text info[INFO_AMOUNT], ipinput, portinput, nickinput, roomnameinput, passwordinput;
sf::Music soundtrack;
sf::RectangleShape yourcolordisplay;
bool bounds[4];//up,right,down,left
bool soundbarexchanged=0, soundpointerpressed=0;
enum modes{ingame, connectroom, lobby};
enum textboxes{none=0, ipbox, portbox, nickbox, roomnamebox, passwordbox, seedbox, listbox, vxmaxbox, vymaxbox, aybox, vjumpbox};
modes mode=connectroom;
textboxes textbox=none;
string buffer, nickname, restofprotocol, protbuffers[3];
size_t received=0;
unsigned int myid=0, to_receive=0, udpto_receive=0, to_ignore=0, udpto_ignore=0, lastgamelistelement=0, frame=0, protbufferi[6], colorpointerpressed=3, choosedmask=0;
float deltagamelist=120;


//zmienne do lobby
string lobbyname;
sf::Text lobbynamet, ainfo[4], seedinput, vxmaxtext, vymaxtext, aytext, vjumptext;
sf::Texture lobbyoutt, checkboxon, checkboxoff, ready1, ready2, advancedt;
sf::Sprite  lobbyouts, cb2players, cb3players, cb4players, oksettings, readys, advanceds;
int playersamount=2;
bool ready=0, changingsettings=0, advancedb=0;
list<sf::Vector2u> spawnpoints;


//zmienne do rozgrywki
sf::Color playercolors[4];
sf::Texture backgroundt, wormt[9], clockt[8];
sf::Sprite  backgrounds, clocks;
sf::Image backgroundi;
sf::Vector2f deltabg(0,0);
sf::Text turntimet;
float mapscale=0.2;
unsigned int turntime=0, clockframe=0, no18delta=0;

//fizyka
int vxmax=2, vymax=875, ay=437, vjump=-300;
sf::Clock clocker;
sf::Time lastittime, currentittime, starttime;
bool started=0;

sf::Image loadMap(string track, list<sf::Vector2u> &spawnpoints){
    spawnpoints.clear();
    fstream plik;
    sf::Image output;
    if(!track.length()){
        cout<<"loadMap("") called\n";
        return output;
    }
    plik.open(track, ios::in | ios::binary);
    if(plik.good()){
        unsigned char ch, structure=0, to_load=0;
        int width=0, height=0, uintbuffer[8];
        sf::Color solid(80, 100, 0);
        for(int i=0; i<4; i++){
            if(!(plik>>noskipws>>ch)){
                cout<<track<<"`s metadata broken\n";
                return output;
            }
            width=width<<8;
            width+=ch;
        }
        for(int i=0; i<4; i++){
            if(!(plik>>noskipws>>ch)){
                cout<<track<<"`s metadata broken\n";
                return output;
            }
            height=height<<8;
            height+=ch;
        }
        output.create(width, height, sf::Color(0,0,0,0));
        while(plik>>noskipws>>ch){
            if(structure==1){
                uintbuffer[(to_load)/4]=uintbuffer[(to_load)/4]<<8;
                uintbuffer[(to_load)/4]+=ch;
                if(!to_load){
                    spawnpoints.push_back(sf::Vector2u(uintbuffer[1], uintbuffer[0]));
                    structure=0;
                }
            }else
            if(structure==2){
                uintbuffer[(to_load)/4]=uintbuffer[(to_load)/4]<<8;
                uintbuffer[(to_load)/4]+=ch;
                if(!to_load){
                    for(int i=0; i<uintbuffer[1]; i++)
                        if(i+uintbuffer[3]<width){
                            for(int j=0; j<uintbuffer[0]; j++)
                                if(j+uintbuffer[2]<height){
                                    output.setPixel(i+uintbuffer[3], j+uintbuffer[2], solid);
                                }else break;
                        }else break;
                    structure=0;
                }
            }

            if(!structure){
                if(ch==1){
                    structure=1;
                    to_load=8;
                }else
                if(ch==2){
                    structure=2;
                    to_load=16;
                }
            }
            to_load--;
        }
        plik.close();
    }else{
        cout<<"plik "<<track<<" is not good\n";
    }

    return output;
}

class gamelistelements{public:
    unsigned int id, pos;
    string name, password;
    sf::Text namet, idt, passwordt;

    gamelistelements(unsigned int posin=1, unsigned int idin=0, string namein="", string passwordin="haslo"){
        pos=posin;
        id=idin;
        name=namein;
        password=passwordin;

        namet.setFont(mainfont);
        namet.setColor(normalclr);
        namet.setCharacterSize(12);
        namet.setString(name);
        idt.setFont(mainfont);
        idt.setColor(normalclr);
        idt.setCharacterSize(12);
        idt.setString(to_string(id));
        passwordt.setFont(mainfont);
        passwordt.setColor(normalclr);
        passwordt.setCharacterSize(12);
        passwordt.setString(password);
    }

    gamelistelements operator =(gamelistelements &input){
        input.id=id;
        input.name=name;
        input.pos=pos;
        input.password=password;
        input.namet.setString(name);
        input.idt.setString(to_string(id));
        input.passwordt.setString(password);
    }

    void update(){
        namet.setString(name);
        idt.setString(to_string(id));
        passwordt.setString(password);
    }

    void draw(sf::RenderWindow &window, sf::Sprite &sprite, sf::Sprite &bsprite){
        sprite.setPosition(0, deltagamelist+pos*sprite.getLocalBounds().height);
        window.draw(sprite);
        bsprite.setPosition(sprite.getLocalBounds().width, deltagamelist+pos*sprite.getLocalBounds().height);
        window.draw(bsprite);
        sprite.setPosition(bsprite.getLocalBounds().width+sprite.getLocalBounds().width, deltagamelist+pos*sprite.getLocalBounds().height);
        window.draw(sprite);
        passwordt.setPosition(bsprite.getLocalBounds().width+sprite.getLocalBounds().width+8, deltagamelist+pos*sprite.getLocalBounds().height+8);
        window.draw(passwordt);
        namet.setPosition(sprite.getLocalBounds().width+8, deltagamelist+pos*sprite.getLocalBounds().height+8);
        window.draw(namet);
        idt.setPosition(8, deltagamelist+pos*sprite.getLocalBounds().height+8);
        window.draw(idt);
    }
};
list<gamelistelements> gamelist;
gamelistelements *gamelistpointer;

class player;

class worm{public:
    sf::Vector2f position, V;
    unsigned int hp, team, id, animcount;
    int direction;
    sf::Sprite sprite, wormmask;
    sf::Text text;
    bool walking;

    worm(sf::Vector2f positionin=sf::Vector2f(0,0), unsigned int teamin=0, unsigned int hpin=200, unsigned int idin=0){
        position=positionin;
        team=teamin;
        hp=hpin;
        id=idin;
        direction=1;
        animcount=0;
        sprite.setTexture(wormt[0]);
        sprite.setPosition((deltabg+position)*mapscale);
        sprite.setScale(mapscale, mapscale);
        text.setFont(mainfont);
        text.setColor(normalclr);
        text.setString("HP="+to_string(hp));
        text.setPosition((deltabg+position+sf::Vector2f(0,-20))*mapscale);
        text.setScale(mapscale, mapscale);
        text.setCharacterSize(20);
        V=sf::Vector2f(0,0);
        walking=0;
    }

    worm operator =(worm input);

    void draw(sf::RenderWindow &window){
        if(hp<=0)return;
        window.draw(sprite);
        window.draw(wormmask);
        window.draw(text);
    }

    void next_anim(){
        animcount++;
        if(animcount>9){
            animcount=0;
        }
        sprite.setTexture(wormt[animcount]);
    }

    void update(){
        sprite.setPosition((deltabg+position)*mapscale);
        sprite.setScale(mapscale*direction, mapscale);
        wormmask.setPosition((deltabg+position+sf::Vector2f(12, -1))*mapscale);
        wormmask.setScale(mapscale*direction, mapscale);
        if(direction==-1){
            sprite.move(sprite.getLocalBounds().width*mapscale, 0);
            wormmask.setPosition(sf::Vector2f((sprite.getPosition().x-12*mapscale), (sprite.getPosition().y-1*mapscale)));
        }
        text.setPosition((deltabg+position+sf::Vector2f(0,-20))*mapscale);
        text.setScale(mapscale, mapscale);
        text.setString("HP="+to_string(hp));
    }
};
vector<worm*> wormpointers;
worm *currentworm=0;

class player{public:
    worm worms[5];
    unsigned char id, hp, emptyworm, mask;
    string name;
    sf::Text namet;
    sf::Color color;
    sf::Texture hpbart;
    sf::Sprite hpbars;

    player(unsigned int idin=0, string namein="guest", sf::Color colorin=playercolors[0], unsigned char maskin=1){
        id=idin;
        name=namein;
        color=colorin;
        mask=maskin;
        hp=0;
        emptyworm=0;
        namet.setString(namein);
        namet.setCharacterSize(12);
        namet.setColor(color);
        namet.setPosition(0,id*40);

        sf::Image image;
        image.create(1,1,color);
        hpbart.loadFromImage(image);
        hpbars.setTexture(hpbart,1);
        hpbars.setScale(hp, 10);
        hpbars.setPosition(0, 30+40*id);
    }

    bool addworm(worm newworm){
        if(emptyworm>4)return 0;
        worms[emptyworm]=newworm;
        hp+=newworm.hp;
        emptyworm++;
        return 1;
    }

    void draw(sf::RenderWindow &window){
        window.draw(hpbars);
        window.draw(namet);
        for(int i=0; i<5; i++){
            worms[i].draw(window);
        }
    }

    player operator =(player input){
        for(int i=0; i<5; i++){
            worms[i]=input.worms[i];
        }
        id       =input.id;
        hp       =input.hp;
        emptyworm=input.emptyworm;
        mask     =input.mask;
        name     =input.name;
        color    =input.color;
        hpbart   =input.hpbart;
        hpbars   =input.hpbars;
        return input;
    }
};
vector<player> players;

worm worm::operator =(worm input){
    position =input.position;
    hp       =input.hp;
    direction=input.direction;
    id       =input.id;
    sprite   =input.sprite;
    text     =input.text;
    team     =input.team;
    wormmask.setTexture(maskt[players[team].mask], 1);
    return input;
}

void placek(sf::Image &image, int x, int y,unsigned int r){
    sf::Vector2f margins;
    margins.x=image.getSize().x;
    margins.y=image.getSize().y;
    for(int i=x-r; i<=x+r; i++){
        if(i>=margins.x)break;
        if(i>=0){
            for(int j=y-r; j<=y+r; j++){
                if(j>=margins.y)break;
                if(j>=0){
                    if(fabs(i-x)*fabs(i-x)+fabs(j-y)*fabs(j-y)<=r*r)image.setPixel(i, j, sf::Color(80,100,0));
                }
            }
        }
    }cout<<"\n";
}

void createmap(unsigned int seed){
    if(seed==1){
        backgroundt.loadFromImage(loadMap("1.map", spawnpoints));
        backgrounds.setTexture(backgroundt);
    }else{
        srand(seed);
        backgroundi.create(6000, 3600, sf::Color(0,0,0,0));
        for(int i=0; i<6000; i++){
            for(int j=3580; j<3600; j++){
                backgroundi.setPixel(i,j, sf::Color(255,0,0));
            }
        }
        {
            int j=3600, j2=3600;
            for(int i=1000; i<=6000; i+=5){
                if(!(rand()%300)){
                    if((rand()%2)&&(j<3100))j+=400;
                    else        j-=400;
                }
                if(i<rand()%2000+2000)j2=-fabs(rand()%8-3)+1;
                else                  j2=fabs(rand()%8-3)-1;
                for(int k=i; k<i+5; k++){
                    for(int n=j+j2/-(k-i-5); ((n<3600)&&(n>=0)); n++){
                        backgroundi.setPixel(k, n, sf::Color(80, 100, 0));
                    }
                }
                j+=j2;
            }
        }
        for(int i=0; i<rand()%100+20; i++){
            placek(backgroundi,(rand()%6000), (rand()%3600), (rand()%200+40));
        }
        backgroundt.loadFromImage(backgroundi);
        backgrounds.setTexture(backgroundt);
    }
}

bool colide(sf::Vector2f pixelin, sf::Image &imagein){
    if((pixelin.x<=imagein.getSize().x-1)&&(pixelin.x>=0)&&(pixelin.y>=0)&&(pixelin.y<=imagein.getSize().y-1)&&(imagein.getPixel(pixelin.x, pixelin.y)==sf::Color(0,0,0,0)))
        return 0;
    else
        return 1;
}

void protocol1(string buffer){
    if(buffer.length()<=20){
        if(connected){
            unsigned char to_send[21]={0};
            to_send[0]=1;
            for(int i=0; i<buffer.length(); i++){
                to_send[i+1]=buffer[i];
            }
            if(clientsocket.send(to_send, 21)==sf::Socket::Done) cout<<"poszlo 0x1\n"; else cout<<"sending error\n";
        }else cout<<"not connected, cannot get nick\n";
    }else cout<<"nick must have no more than 20 letters\n";
}

bool protocol3(){
    if(connected){
        unsigned char to_send[1];
        to_send[0]=0x3;
        if(clientsocket.send(to_send, 1)==sf::Socket::Done){
            cout<<"room leaved\n";
            mode=connectroom;
            return 1;
        }else cout<<"sending error 0x3\n";
    }else cout<<"not connected, cannot leave room\n";
    return 0;
}

void protocol6(){
    unsigned char to_send[7];
    to_send[0]=6;
    protbufferi[0]=myid;
    for(int i=4; i>0; i--){
        to_send[i]=protbufferi[0]%256;
        protbufferi[0]=protbufferi[0]>>8;
    }
    to_send[5]=(udpsocket.getLocalPort()>>8)%256;
    to_send[6]=udpsocket.getLocalPort()%256;
    if(udpsocket.send(to_send, 5, serverip, 31337)!=sf::Socket::Done){
        cout<<"sending error 0x6\n";
    }
    protbufferi[0]=0;
}

void protocol10(){
    if(connected){
        unsigned char to_send[1];
        to_send[0]=0x10;
        if(clientsocket.send(to_send, 1)==sf::Socket::Done){
            cout<<"poszlo 0x10\n";
            gamelist.clear();
            lastgamelistelement=0;
            deltagamelist=120;
        }else cout<<"sending error 0x10\n";
    }else cout<<"not connected, cannot get room list\n";
}

void protocol20(string buffer, string buffer2){
    if(connected){
        unsigned char to_send[22+(buffer.length())];
        to_send[0]=0x20;
        for(int i=1; i<21; i++){
            if(i-1<buffer2.length()){
                to_send[i]=buffer2[i-1];
            }else
                to_send[i]=0;
        }
        to_send[21]=buffer.length();
        for(int i=22; i<to_send[21]+22; i++){
            to_send[i]=buffer[i-22];
        }
        if(clientsocket.send(to_send, 22+(buffer.length()))==sf::Socket::Done) cout<<"poszlo 0x20\n";
        else cout<<"sending error\n";
    }else cout<<"not connected, can not create room\n";
}

void protocol22(){
    if(connected){
        unsigned char to_send[9];
        to_send[0]=0x22;
        to_send[1]=(ay>>8)%256;
        to_send[2]=ay%256;
        to_send[3]=((-vjump)>>8)%256;
        to_send[4]=(-vjump)%256;
        to_send[5]=(vymax>>8)%256;
        to_send[6]=(vymax)%256;
        to_send[7]=(vxmax>>8)%256;
        to_send[8]=(vxmax)%256;
        if(clientsocket.send(to_send, 9)==sf::Socket::Done){
            cout<<"poszlo 0x22\n";
        }else cout<<"sending error 0x22\n";
    }else cout<<"not connected, cannot set physic\n";
}

void protocol24(unsigned int seed, unsigned char playersamount){
    if(connected){
		unsigned char to_send[6];
		to_send[0]=0x24;
		for(int i=4; i>0; i--){
			to_send[i]=seed%256;
			seed=seed>>8;
		}
		to_send[5]=playersamount;
		if(clientsocket.send(to_send, 6)==sf::Socket::Done) cout<<"poszlo 0x24\n";
		else cout<<"sending error\n";
    }else cout<<"not connected, cannot change settings\n";
}

void protocol26(unsigned int gameid, string password){
    if(password.length()<256){
        if(connected){
            unsigned char length=password.length(), to_send[10+length];
            to_send[0]=0x26;
            protbufferi[0]=gameid;
            for(int i=4; i>0; i--){
                to_send[i]=gameid%256;
                gameid=gameid>>8;
            }
            protbuffers[0]=password;
            to_send[5]=length;
            for(int i=0; i<length; i++){
                to_send[i+6]=password[i];
            }
            if(clientsocket.send(to_send, 6+length)==sf::Socket::Done){
                cout<<"poszlo 0x26\n";
            }else cout<<"sending error 0x26\n";
        }else cout<<"not connected, cannot join\n";
    }else cout<<"password too long (255 chars max)\n";
}

void protocol28(){
    if(connected){
        unsigned char to_send[1];
        to_send[0]=0x28;
        if(clientsocket.send(to_send, 1)==sf::Socket::Done){
            cout<<"poszlo 0x28\n";
        }else cout<<"sending error 0x28\n";
    }else cout<<"not connected, cannot get room settings\n";
}

void protocol2a(sf::Color input, unsigned char maskid){
    if(connected){
        unsigned char to_send[5];
        to_send[0]=0x2a;
        to_send[1]=input.r;
        to_send[2]=input.g;
        to_send[3]=input.b;
        to_send[4]=maskid;
        if(clientsocket.send(to_send, 1)==sf::Socket::Done){
            cout<<"poszlo 0x2a\n";
        }else cout<<"sending error 0x2a\n";
    }else cout<<"not connected, cannot set player settings\n";
}

void protocol2c(){
    if(connected){
        unsigned char to_send[1];
        to_send[0]=0x2c;
        if(clientsocket.send(to_send, 1)==sf::Socket::Done){
            cout<<"poszlo 0x2c\n";
        }else cout<<"sending error 0x2c\n";
    }else cout<<"not connected, cannot get ready\n";
}

void protocol2e(){
    if(connected){
        unsigned char to_send[1];
        to_send[0]=0x2e;
        if(clientsocket.send(to_send, 1)==sf::Socket::Done){
            cout<<"poszlo 0x2e\n";
        }else cout<<"sending error 0x2e\n";
    }else cout<<"not connected, cannot get players list\n";
}

void protocol31(){
    if(connected){
        unsigned char to_send[3];
        to_send[0]=0x31;
        to_send[1]=(udpport>>8)%256;
        to_send[2]=udpport%256;
        if(clientsocket.send(to_send, 3)==sf::Socket::Done){
        }else cout<<"sending error 0x31\n";
    }else cout<<"not connected, cannot get turn time\n";
}

bool protocol37(){return 1;
    if(connected){
        unsigned char to_send[1];
        to_send[0]=0x37;
        if(clientsocket.send(to_send, 1)==sf::Socket::Done){return 1;
        }else cout<<"sending error 0x37\n";
    }else cout<<"not connected, cannot jump\n";
    return 0;
}

bool protocol38(){//return 1;
    if(connected){
        unsigned char to_send[1];
        to_send[0]=0x38;
        if(udpsocket.send(to_send, 1, serverip, udpport)==sf::Socket::Done){
            udpport=udpsocket.getLocalPort();
            serverip=clientsocket.getRemoteAddress();
            return 1;
        }else cout<<"sending error 0x38\n";
    }else cout<<"not connected, can not move\n";
        udpport=udpsocket.getLocalPort();
        serverip=clientsocket.getRemoteAddress();
    return 0;
}

bool protocol39(){return 1;
    if(connected){
        unsigned char to_send[1];
        to_send[0]=0x39;
        if(clientsocket.send(to_send, 1)==sf::Socket::Done){return 1;
        }else cout<<"sending error 0x39\n";
    }else cout<<"not connected, can not move\n";
    return 0;
}
//}

void save(string track, string input){
    if((!input.length())||(!track.length()))return;
    fstream plik;
    plik.open(track, ios::out | ios::binary);
    if(plik.good()) plik<<input;
    else cout<<"plik not good\n";
    plik.close();
}

int main(){
    {
        system("color 0a");
        window.setFramerateLimit(60);
        playercolors[0]=sf::Color(0,255,0);
        playercolors[1]=sf::Color(255,255,0);
        playercolors[2]=sf::Color(0,255,255);
        playercolors[3]=sf::Color(255,0,0);
        soundtrack.openFromFile("snd/music.ogg");
        soundtrack.setVolume(0);
        soundtrack.setLoop(1);
        soundtrack.play();
        soundicont.loadFromFile("img/speaker.bmp");
        soundicons.setTexture(soundicont);
        soundicons.setPosition(1140,0);
        soundbart.loadFromFile("img/sndbar.bmp");
        soundbars.setTexture(soundbart);
        soundbars.setPosition(1150,30);
        soundpointert.loadFromFile("img/sndpointer.png");
        soundpointers.setTexture(soundpointert);
        soundpointers.setPosition(1139,24);
        inputbart.loadFromFile("img/inputbar.bmp");
        inputbars.setTexture(inputbart);
        binputbart.loadFromFile("img/binputbar.bmp");
        binputbars.setTexture(binputbart);
        okt.loadFromFile("img/ok.bmp");
        okconnects.setTexture(okt);
        okconnects.setPosition(0,60);
        oknicks.setTexture(okt);
        oknicks.setPosition(200,30);
        okcreaterooms.setTexture(okt);
        okcreaterooms.setPosition(500,60);
        oksettings.setTexture(okt);
        oksettings.setPosition(0,180);
        okrgbm.setTexture(okt);
        okrgbm.setPosition(910,60);
        ont.loadFromFile("img/on.bmp");
        offt.loadFromFile("img/off.bmp");
        connectionS.setTexture(offt);
        connectionS.setPosition(1170,0);
        bgplanei.create(1, 1, bgcolor);
        bgplanet.loadFromImage(bgplanei);
        bgplanes.setTexture(bgplanet);
        bgplanes.setScale(1200, 120);
        bgplanes.setPosition(0,0);
        reloadgamelistt.loadFromFile("img/reload.bmp");
        reloadgamelists.setTexture(reloadgamelistt);
        reloadgamelists.setPosition(54, 60);
        lobbyoutt.loadFromFile("img/back.bmp");
        lobbyouts.setTexture(lobbyoutt);
        lobbyouts.setPosition(0,0);
        checkboxon.loadFromFile("img/checkboxon.bmp");
        checkboxoff.loadFromFile("img/checkboxoff.bmp");
        cb2players.setTexture(checkboxon);
        cb2players.setPosition(0,98);
        cb3players.setTexture(checkboxoff);
        cb3players.setPosition(0,128);
        cb4players.setTexture(checkboxoff);
        cb4players.setPosition(0,158);
        ready1.loadFromFile("img/ready1.bmp");
        ready2.loadFromFile("img/ready2.bmp");
        readys.setTexture(ready2);
        readys.setPosition(200,30);
        advancedt.loadFromFile("img/advanced.bmp");
        advanceds.setTexture(advancedt);
        advanceds.setPosition(270,30);
        clocks.setPosition(0,0);
        for(int i=0; i<9; i++){
            wormt[i].loadFromFile("img/rob2.0/anim"+to_string(i+1)+".png");
        }
        for(int i=0; i<8; i++){
            clockt[i].loadFromFile("img/clock/clock"+to_string(i+1)+".png");
        }

        mainfont.loadFromFile("font.ttf");
        ipinput.setFont(mainfont);
        ipinput.setString("185.84.136.151");
        ipinput.setCharacterSize(12);
        ipinput.setPosition(8,8);
        ipinput.setColor(normalclr);
        portinput.setFont(mainfont);
        portinput.setString("31337");
        portinput.setCharacterSize(12);
        portinput.setPosition(8,38);
        portinput.setColor(normalclr);
        nickinput.setFont(mainfont);
        nickinput.setString("guest");
        nickinput.setCharacterSize(12);
        nickinput.setPosition(208,8);
        nickinput.setColor(normalclr);
        roomnameinput.setFont(mainfont);
        roomnameinput.setString("room001");
        roomnameinput.setCharacterSize(12);
        roomnameinput.setPosition(508,8);
        roomnameinput.setColor(normalclr);
        passwordinput.setFont(mainfont);
        passwordinput.setString("123");
        passwordinput.setCharacterSize(12);
        passwordinput.setPosition(508,38);
        passwordinput.setColor(normalclr);
        lobbynamet.setFont(mainfont);
        lobbynamet.setString("");
        lobbynamet.setCharacterSize(16);
        lobbynamet.setColor(normalclr);
        lobbynamet.setPosition(38,8);
        seedinput.setFont(mainfont);
        seedinput.setString("1");
        seedinput.setCharacterSize(12);
        seedinput.setColor(normalclr);
        seedinput.setPosition(8,38);
        turntimet.setFont(mainfont);
        turntimet.setString("0s");
        turntimet.setCharacterSize(25);
        turntimet.setColor(normalclr);
        turntimet.setPosition(40,0);
        colorbart[0].loadFromFile("img/redbar.bmp");
        colorbart[1].loadFromFile("img/greenbar.bmp");
        colorbart[2].loadFromFile("img/bluebar.bmp");
        colorpointert.loadFromFile("img/colorpointer.png");
        for(int i=0; i<3; i++){
            colorbars[i].setTexture(colorbart[i]);
            colorbars[i].setPosition(800, 25+25*i);
            colorpointers[i].setTexture(colorpointert);
            colorpointers[i].setPosition(796, 15+25*i);
        }
        for(int i=0; i<INFO_AMOUNT; i++){
            info[i].setFont(mainfont);
            info[i].setCharacterSize(12);
            info[i].setColor(normalclr);
        }
        yourcolordisplay.setPosition(910, 10);
        yourcolordisplay.setSize(sf::Vector2f(50,50));
        yourcolordisplay.setFillColor(yourcolor);
        masklt.loadFromFile("img/left.bmp");
        maskrt.loadFromFile("img/right.bmp");
        maskls.setTexture(masklt);
        maskrs.setTexture(maskrt);
        maskls.setPosition(960, 25);
        maskrs.setPosition(1050, 25);
        for(int i=0; i<10; i++){
            maskchooset[i].loadFromFile("img/mask/"+to_string(i)+".png");
        }
        for(int i=0; i<10; i++){
            maskt[i].loadFromFile("img/mask_ingame/"+to_string(i)+".png");
        }
        maskchoose.setTexture(maskchooset[choosedmask]);
        maskchoose.setPosition(990, 10);
        info[0].setString("ip");
        info[0].setPosition(150,8);
        info[1].setString("port");
        info[1].setPosition(150,38);
        info[2].setString("nickname");
        info[2].setPosition(400, 8);
        info[3].setString("roomname");
        info[3].setPosition(700, 8);
        info[4].setString("password");
        info[4].setPosition(700, 38);
        info[5].setString("game id");
        info[5].setPosition(8, 98);
        info[6].setString("game name");
        info[6].setPosition(158, 98);
        info[7].setString("seed");
        info[7].setPosition(150,38);
        info[8].setString("players");
        info[8].setPosition(0,68);
        info[9].setString("2");
        info[9].setPosition(20,98);
        info[10].setString("3");
        info[10].setPosition(20,128);
        info[11].setString("4");
        info[11].setPosition(20,158);
        fstream plik;
        plik.open("physic.cfg", ios::in | ios::binary);
        if(plik.good()){
            string line="";
            int linenumber=0;
            while(getline(plik, line)){
                linenumber++;
                switch(linenumber){
                    case 1:{
                        ay=atof(line.c_str());
                    }break;
                    case 2:{
                        vymax=atof(line.c_str());
                    }break;
                    case 3:{
                        vjump=atof(line.c_str());
                    }break;
                    case 4:{
                        vxmax=atof(line.c_str());
                    }break;
                    default:{
                        cout<<"unspecified physic.cfg line"<<linenumber<<"\n";
                    }break;
                }
            }
        }else{
            cout<<"physic.cfg does not exist or is damaged\n";
        }
        vxmaxtext.setFont(mainfont);
        vxmaxtext.setString(to_string(vxmax));
        vxmaxtext.setCharacterSize(12);
        vxmaxtext.setColor(normalclr);
        vxmaxtext.setPosition(278,68);
        vymaxtext.setFont(mainfont);
        vymaxtext.setString(to_string(vymax));
        vymaxtext.setCharacterSize(12);
        vymaxtext.setColor(normalclr);
        vymaxtext.setPosition(278,98);
        aytext.setFont(mainfont);
        aytext.setString(to_string(ay));
        aytext.setCharacterSize(12);
        aytext.setColor(normalclr);
        aytext.setPosition(278,128);
        vjumptext.setFont(mainfont);
        vjumptext.setString(to_string(vjump));
        vjumptext.setCharacterSize(12);
        vjumptext.setColor(normalclr);
        vjumptext.setPosition(278,158);
        for(int i=0; i<4; i++){
            ainfo[i].setFont(mainfont);
            ainfo[i].setCharacterSize(12);
            ainfo[i].setColor(normalclr);
        }
        ainfo[0].setString("maxV(x)");
        ainfo[0].setPosition(278+inputbart.getSize().x, 68);
        ainfo[1].setString("maxV(y)");
        ainfo[1].setPosition(278+inputbart.getSize().x, 98);
        ainfo[2].setString("a(y)");
        ainfo[2].setPosition(278+inputbart.getSize().x, 128);
        ainfo[3].setString("jump");
        ainfo[3].setPosition(278+inputbart.getSize().x, 158);
/*
        backgroundt.loadFromImage(backgroundi=loadMap("1.map", spawnpoints));
        backgrounds.setTexture(backgroundt, 1);
        backgrounds.setScale(0.2,0.2);
        mode=ingame;
        playersamount=1;
        players.push_back(player());
        players[0].addworm(worm());
        players[0].addworm(worm(sf::Vector2f(200,0), 0, 200, 1));
        wormpointers.push_back(&players[0].worms[0]);
        started=1;
        currentworm=wormpointers[0];*/
    }
    while(window.isOpen()){
        while(window.pollEvent(event)){
            if(event.type==sf::Event::Closed){
                if(connected)
                    clientsocket.disconnect();
                window.close();
            }else
            if(event.type==sf::Event::Resized){
                window.setSize(sf::Vector2u(1200, 720));
            }
            if(mode==ingame){
                if(event.type==sf::Event::MouseWheelMoved){
                    if(event.mouseWheel.delta>0){
                        if(mapscale<1){
                            mapscale+=0.1;
                            backgrounds.setScale(mapscale, mapscale);
                            backgrounds.setPosition(deltabg*mapscale);
                            for(int i=0; i<wormpointers.size(); i++){
                                (*wormpointers[i]).update();
                            }
                        }
                    }else{
                        if(mapscale>0.3){
                            mapscale-=0.1;
                            backgrounds.setScale(mapscale, mapscale);
                            backgrounds.setPosition(deltabg*mapscale);
                            for(int i=0; i<wormpointers.size(); i++){
                                (*wormpointers[i]).update();
                            }
                        }
                    }
                }else
                if(event.type==sf::Event::MouseMoved){
                    bounds[0]=bounds[1]=bounds[2]=bounds[3]=0;
                    if(event.mouseMove.x<10) bounds[3]=1; else
                    if(event.mouseMove.x>window.getSize().x-10) bounds[1]=1;
                    if(event.mouseMove.y<10) bounds[0]=1; else
                    if(event.mouseMove.y>window.getSize().y-10) bounds[2]=1;
                }else
                if(event.type==sf::Event::TextEntered){
                    if(event.text.unicode==32){//space
                        if(currentworm){
                            bool fhasfloor=0;
                            for(int i=0; i<wormt[0].getSize().x; i++){
                                if(colide(sf::Vector2f(i+(*currentworm).position.x, (*currentworm).position.y+wormt[0].getSize().y), backgroundi)){
                                    fhasfloor=1;
                                }
                            }
                            if(fhasfloor){
                                if(protocol37()){
                                    (*currentworm).V.y+=vjump;
                                    (*currentworm).walking=0;
                                    (*currentworm).V.x=0;
                                    (*currentworm).animcount=0;
                                    (*currentworm).sprite.setTexture(wormt[0]);
                                }
                            }
                        }
                    }
                }else
                if((event.type==sf::Event::KeyPressed)||(event.type==sf::Event::KeyReleased)){
                    if(event.key.code==sf::Keyboard::A){
                        if((currentworm)&&((*currentworm).V.x<=0)&&(((event.type==sf::Event::KeyPressed)&&(!(*currentworm).walking))||((event.type==sf::Event::KeyReleased)&&((*currentworm).walking)))){
                            if(protocol38()){
                                if(!(*currentworm).walking){
                                    (*currentworm).V.x=-vxmax;
                                    (*currentworm).walking=1;
                                    (*currentworm).direction=-1;
                                }else{
                                    (*currentworm).walking=0;
                                    (*currentworm).V.x=0;
                                    (*currentworm).animcount=0;
                                    (*currentworm).sprite.setTexture(wormt[0]);
                                }
                            }
                        }
                    }else
                    if(event.key.code==sf::Keyboard::D){
                        if((currentworm)&&((*currentworm).V.x>=0)&&(((event.type==sf::Event::KeyPressed)&&(!(*currentworm).walking))||((event.type==sf::Event::KeyReleased)&&((*currentworm).walking)))){
                            if(protocol39()){
                                if(!(*currentworm).walking){
                                    (*currentworm).V.x=vxmax;
                                    (*currentworm).walking=1;
                                    (*currentworm).direction=1;
                                }else{
                                    (*currentworm).walking=0;
                                    (*currentworm).V.x=0;
                                    (*currentworm).animcount=0;
                                    (*currentworm).sprite.setTexture(wormt[0]);
                                }
                            }
                        }
                    }
                }
            }else
            if(mode==connectroom){
                if(event.type==sf::Event::MouseButtonPressed){
                    if(textbox==ipbox){
                        ipinput.setColor(normalclr);
                    }else
                    if(textbox==portbox){
                        portinput.setColor(normalclr);
                    }else
                    if(textbox==nickbox){
                        nickinput.setColor(normalclr);
                    }else
                    if(textbox==roomnamebox){
                        roomnameinput.setColor(normalclr);
                    }else
                    if(textbox==passwordbox){
                        passwordinput.setColor(normalclr);
                    }else
                    if(textbox==listbox){
                        (*gamelistpointer).passwordt.setColor(normalclr);
                        gamelistpointer=0;
                    }
                    textbox=none;

                    if((event.mouseButton.y<=120)||(event.mouseButton.x>1100)){
                        if((event.mouseButton.x>=ipinput.getPosition().x-8)&&(event.mouseButton.x<=ipinput.getPosition().x-8+inputbars.getLocalBounds().width)&&(event.mouseButton.y>=ipinput.getPosition().y-8)&&(event.mouseButton.y<=ipinput.getPosition().y-8+inputbars.getLocalBounds().height)){
                            textbox=ipbox;
                            ipinput.setColor(checkedclr);
                        }else
                        if((event.mouseButton.x>=portinput.getPosition().x-8)&&(event.mouseButton.x<=portinput.getPosition().x-8+inputbars.getLocalBounds().width)&&(event.mouseButton.y>=portinput.getPosition().y-8)&&(event.mouseButton.y<=portinput.getPosition().y-8+inputbars.getLocalBounds().height)){
                            textbox=portbox;
                            portinput.setColor(checkedclr);
                        }else
                        if((event.mouseButton.x>=nickinput.getPosition().x-8)&&(event.mouseButton.x<=nickinput.getPosition().x-8+binputbars.getLocalBounds().width)&&(event.mouseButton.y>=nickinput.getPosition().y-8)&&(event.mouseButton.y<=nickinput.getPosition().y-8+binputbars.getLocalBounds().height)){
                            textbox=nickbox;
                            nickinput.setColor(checkedclr);
                        }else
                        if((event.mouseButton.x>=roomnameinput.getPosition().x-8)&&(event.mouseButton.x<=roomnameinput.getPosition().x-8+binputbars.getLocalBounds().width)&&(event.mouseButton.y>=roomnameinput.getPosition().y-8)&&(event.mouseButton.y<=roomnameinput.getPosition().y-8+binputbars.getLocalBounds().height)){
                            textbox=roomnamebox;
                            roomnameinput.setColor(checkedclr);
                        }else
                        if((event.mouseButton.x>=passwordinput.getPosition().x-8)&&(event.mouseButton.x<=passwordinput.getPosition().x-8+binputbars.getLocalBounds().width)&&(event.mouseButton.y>=passwordinput.getPosition().y-8)&&(event.mouseButton.y<=passwordinput.getPosition().y-8+binputbars.getLocalBounds().height)){
                            textbox=passwordbox;
                            passwordinput.setColor(checkedclr);
                        }else
                        if((event.mouseButton.x>=okconnects.getPosition().x)&&(event.mouseButton.x<=okconnects.getPosition().x+okconnects.getLocalBounds().width)&&(event.mouseButton.y>=okconnects.getPosition().y)&&(event.mouseButton.y<=okconnects.getPosition().y+okconnects.getLocalBounds().height)){
                            if(connect(ipinput.getString(), portinput.getString())){
                                cout<<"connected\n";
                            }
                        }else
                        if((event.mouseButton.x>=oknicks.getPosition().x)&&(event.mouseButton.x<=oknicks.getPosition().x+oknicks.getLocalBounds().width)&&(event.mouseButton.y>=oknicks.getPosition().y)&&(event.mouseButton.y<=oknicks.getPosition().y+oknicks.getLocalBounds().height)){
                            protocol1(nickinput.getString());
                        }else
                        if((event.mouseButton.x>=okcreaterooms.getPosition().x)&&(event.mouseButton.x<=okcreaterooms.getPosition().x+okcreaterooms.getLocalBounds().width)&&(event.mouseButton.y>=okcreaterooms.getPosition().y)&&(event.mouseButton.y<=okcreaterooms.getPosition().y+okcreaterooms.getLocalBounds().height)){
                            protocol20(passwordinput.getString(), roomnameinput.getString());
                        }else
                        if((event.mouseButton.x>=soundpointers.getPosition().x)&&(event.mouseButton.x<=soundpointers.getPosition().x+soundpointers.getLocalBounds().width)&&(event.mouseButton.y>=soundpointers.getPosition().y)&&(event.mouseButton.y<=soundpointers.getPosition().y+soundpointers.getLocalBounds().height)){
                            soundpointerpressed=1;
                        }else
                        if((event.mouseButton.x>=soundicons.getPosition().x)&&(event.mouseButton.x<=soundicons.getPosition().x+soundicons.getLocalBounds().width)&&(event.mouseButton.y>=soundicons.getPosition().y)&&(event.mouseButton.y<=soundicons.getPosition().y+soundicons.getLocalBounds().height)){
                            soundbarexchanged=!soundbarexchanged;
                        }else
                        if((event.mouseButton.x>=reloadgamelists.getPosition().x)&&(event.mouseButton.x<=reloadgamelists.getPosition().x+reloadgamelists.getLocalBounds().width)&&(event.mouseButton.y>=reloadgamelists.getPosition().y)&&(event.mouseButton.y<=reloadgamelists.getPosition().y+reloadgamelists.getLocalBounds().height)){
                            if(connected){protocol10();}
                        }
                    }else{
                        int listelementbuffer=int(event.mouseButton.y-deltagamelist)/int(inputbars.getLocalBounds().height);
                        if((listelementbuffer<=lastgamelistelement)&&(listelementbuffer>0)){
                            if(event.mouseButton.x<=inputbars.getLocalBounds().width+binputbars.getLocalBounds().width){
                                for(list<gamelistelements>::iterator i=gamelist.begin(); i!=gamelist.end(); ++i){
                                    if((*i).pos==listelementbuffer){
                                        protocol26((*i).id, (*i).password);
                                        break;
                                    }
                                }
                            }else
                            if(event.mouseButton.x<=inputbars.getLocalBounds().width*2+binputbars.getLocalBounds().width){
                                textbox=listbox;
                                for(list<gamelistelements>::iterator i=gamelist.begin(); i!=gamelist.end(); ++i){
                                    if((*i).pos==listelementbuffer){
                                        gamelistpointer=&(*i);
                                        (*gamelistpointer).passwordt.setColor(checkedclr);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }else
                if(event.type==sf::Event::TextEntered){sf::Text* inputpointer=0;
                    if(textbox==ipbox){inputpointer=&ipinput;
                        if((event.text.unicode==13)||(event.text.unicode==9)){
                            textbox=portbox;
                            portinput.setColor(checkedclr);
                            ipinput.setColor(normalclr);
                        }
                    }else
                    if(textbox==portbox){inputpointer=&portinput;
                        if(event.text.unicode==9){
                            textbox=ipbox;
                            portinput.setColor(normalclr);
                            ipinput.setColor(checkedclr);
                        }else
                        if(event.text.unicode==13){
                            if(connect(ipinput.getString(), portinput.getString())) cout<<"connected\n";
                        }
                    }else
                    if(textbox==nickbox){inputpointer=&nickinput;
                        if(event.text.unicode==13){
                            protocol1(nickinput.getString());
                            nickinput.setColor(normalclr);
                            inputpointer=0;
                        }
                    }else
                    if(textbox==roomnamebox){inputpointer=&roomnameinput;
                        if((event.text.unicode==13)&&(event.text.unicode==9)){
                            textbox=passwordbox;
                            passwordinput.setColor(checkedclr);
                            roomnameinput.setColor(normalclr);
                        }
                    }else
                    if(textbox==passwordbox){inputpointer=&passwordinput;
                        if(event.text.unicode==13){
                            protocol20(passwordinput.getString(), roomnameinput.getString());
                            inputpointer=0;
                        }
                    }else
                    if(textbox==listbox){
                        if(event.text.unicode==13){
                            (*gamelistpointer).passwordt.setColor(normalclr);
                            textbox=none;
                        }else
                        if(event.text.unicode==8){
                            if((*gamelistpointer).password.length()){
                                (*gamelistpointer).password.erase((*gamelistpointer).password.length()-1);
                                (*gamelistpointer).update();
                            }
                        }else
                        if((event.text.unicode==127)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            (*gamelistpointer).password="";
                            (*gamelistpointer).update();
                        }else
                        if((event.text.unicode==3)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            copyToClipboard((*gamelistpointer).password);
                        }else
                        if((event.text.unicode==22)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            if(OpenClipboard(0)){
                                (*gamelistpointer).password=((*gamelistpointer).password+(char*)(GetClipboardData(CF_TEXT)));
                                CloseClipboard();
                                (*gamelistpointer).update();
                            }
                        }else
                        {
                            (*gamelistpointer).password+=event.text.unicode;
                            (*gamelistpointer).update();
                        }
                    }
                    if(inputpointer){
                        if(event.text.unicode==8){
                            buffer=(*inputpointer).getString();
                            if(buffer.length()){
                                buffer.erase(buffer.length()-1);
                                (*inputpointer).setString(buffer);
                            }
                        }else
                        if((event.text.unicode==127)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            (*inputpointer).setString("");
                        }else
                        if((event.text.unicode==3)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            copyToClipboard((*inputpointer).getString());
                        }else
                        if((event.text.unicode==22)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            if(OpenClipboard(0)){
                                (*inputpointer).setString((*inputpointer).getString()+(char*)(GetClipboardData(CF_TEXT)));
                                CloseClipboard();
                            }
                        }else
                        {
                            (*inputpointer).setString((*inputpointer).getString()+event.text.unicode);
                        }
                    }
                }else
                if(event.type==sf::Event::MouseWheelMoved){
                    if(connected)
                        deltagamelist-=event.mouseWheel.delta*10;
                }else
                if(event.type==sf::Event::MouseButtonReleased){
                    soundpointerpressed=0;
                }else
                if(event.type==sf::Event::MouseMoved){
                    if(soundpointerpressed){
                        soundtrack.setVolume((event.mouseMove.y-30)*(event.mouseMove.y>30)-(event.mouseMove.y-130)*(event.mouseMove.y>130));
                        soundpointers.setPosition(1139, 24+soundtrack.getVolume());
                    }
                }
            }else
            if(mode==lobby){
                if(event.type==sf::Event::MouseButtonPressed){
                    if(textbox==seedbox){
                        seedinput.setColor(normalclr);
                    }else
                    if(textbox==vxmaxbox){
                        vxmaxtext.setColor(normalclr);
                        vxmax=atof(vxmaxtext.getString().toAnsiString().c_str());
                    }else
                    if(textbox==vymaxbox){
                        vymaxtext.setColor(normalclr);
                    }else
                    if(textbox==aybox){
                        aytext.setColor(normalclr);
                        ay=atof(aytext.getString().toAnsiString().c_str());
                    }else
                    if(textbox==vjumpbox){
                        vjumptext.setColor(normalclr);
                        vjump=atof(vjumptext.getString().toAnsiString().c_str());
                    }
                    textbox=none;

                    if((event.mouseButton.x>=lobbyouts.getPosition().x)&&(event.mouseButton.x<=lobbyouts.getPosition().x+lobbyouts.getLocalBounds().width)&&(event.mouseButton.y>=lobbyouts.getPosition().y)&&(event.mouseButton.y<=lobbyouts.getPosition().y+lobbyouts.getLocalBounds().height)){
                        protocol3();
                    }else
                    if((event.mouseButton.x>=advanceds.getPosition().x)&&(event.mouseButton.x<=advanceds.getPosition().x+advanceds.getLocalBounds().width)&&(event.mouseButton.y>=advanceds.getPosition().y)&&(event.mouseButton.y<=advanceds.getPosition().y+advanceds.getLocalBounds().height)){
                        advancedb=!advancedb;
                    }else
                    if((event.mouseButton.x>=cb2players.getPosition().x)&&(event.mouseButton.x<=cb2players.getPosition().x+cb2players.getLocalBounds().width)&&(event.mouseButton.y>=cb2players.getPosition().y)&&(event.mouseButton.y<=cb2players.getPosition().y+cb2players.getLocalBounds().height)){
                        switch(playersamount){
                            case 3:
                                cb3players.setTexture(checkboxoff);
                            break;
                            case 4:
                                cb4players.setTexture(checkboxoff);
                            break;
                        }
                        cb2players.setTexture(checkboxon);
                        playersamount=2;
                    }else
                    if((event.mouseButton.x>=cb3players.getPosition().x)&&(event.mouseButton.x<=cb3players.getPosition().x+cb3players.getLocalBounds().width)&&(event.mouseButton.y>=cb3players.getPosition().y)&&(event.mouseButton.y<=cb3players.getPosition().y+cb3players.getLocalBounds().height)){
                        switch(playersamount){
                            case 2:
                                cb2players.setTexture(checkboxoff);
                            break;
                            case 4:
                                cb4players.setTexture(checkboxoff);
                            break;
                        }
                        cb3players.setTexture(checkboxon);
                        playersamount=3;
                    }else
                    if((event.mouseButton.x>=cb4players.getPosition().x)&&(event.mouseButton.x<=cb4players.getPosition().x+cb4players.getLocalBounds().width)&&(event.mouseButton.y>=cb4players.getPosition().y)&&(event.mouseButton.y<=cb4players.getPosition().y+cb4players.getLocalBounds().height)){
                        switch(playersamount){
                            case 2:
                                cb2players.setTexture(checkboxoff);
                            break;
                            case 3:
                                cb3players.setTexture(checkboxoff);
                            break;
                        }
                        cb4players.setTexture(checkboxon);
                        playersamount=4;
                    }else
                    if((changingsettings)&&(event.mouseButton.x>=oksettings.getPosition().x)&&(event.mouseButton.x<=oksettings.getPosition().x+oksettings.getLocalBounds().width)&&(event.mouseButton.y>=oksettings.getPosition().y)&&(event.mouseButton.y<=oksettings.getPosition().y+oksettings.getLocalBounds().height)){
                        if(seedinput.getString().getSize()<10) protocol22();
                        else cout<<"seed is too big\n";
                    }else
                    if((event.mouseButton.x>=readys.getPosition().x)&&(event.mouseButton.x<=readys.getPosition().x+readys.getLocalBounds().width)&&(event.mouseButton.y>=readys.getPosition().y)&&(event.mouseButton.y<=readys.getPosition().y+readys.getLocalBounds().height)){
                        protocol2c();
                    }else
                    if((event.mouseButton.x>=soundpointers.getPosition().x)&&(event.mouseButton.x<=soundpointers.getPosition().x+soundpointers.getLocalBounds().width)&&(event.mouseButton.y>=soundpointers.getPosition().y)&&(event.mouseButton.y<=soundpointers.getPosition().y+soundpointers.getLocalBounds().height)){
                        soundpointerpressed=1;
                    }else
                    if((event.mouseButton.x>=soundicons.getPosition().x)&&(event.mouseButton.x<=soundicons.getPosition().x+soundicons.getLocalBounds().width)&&(event.mouseButton.y>=soundicons.getPosition().y)&&(event.mouseButton.y<=soundicons.getPosition().y+soundicons.getLocalBounds().height)){
                        soundbarexchanged=!soundbarexchanged;
                    }else
                    if((changingsettings)&&(event.mouseButton.x>=seedinput.getPosition().x-8)&&(event.mouseButton.x<=seedinput.getPosition().x-8+inputbars.getLocalBounds().width)&&(event.mouseButton.y>=seedinput.getPosition().y-8)&&(event.mouseButton.y<=seedinput.getPosition().y-8+inputbars.getLocalBounds().height)){
                        seedinput.setColor(checkedclr);
                        textbox=seedbox;
                    }else
                    if((changingsettings)&&(event.mouseButton.x>=vxmaxtext.getPosition().x-8)&&(event.mouseButton.x<=vxmaxtext.getPosition().x-8+inputbars.getLocalBounds().width)&&(event.mouseButton.y>=vxmaxtext.getPosition().y-8)&&(event.mouseButton.y<=vxmaxtext.getPosition().y-8+inputbars.getLocalBounds().height)){
                        vxmaxtext.setColor(checkedclr);
                        textbox=vxmaxbox;
                        vxmax=atof(vxmaxtext.getString().toAnsiString().c_str());
                    }else
                    if((changingsettings)&&(event.mouseButton.x>=vymaxtext.getPosition().x-8)&&(event.mouseButton.x<=vymaxtext.getPosition().x-8+inputbars.getLocalBounds().width)&&(event.mouseButton.y>=vymaxtext.getPosition().y-8)&&(event.mouseButton.y<=vymaxtext.getPosition().y-8+inputbars.getLocalBounds().height)){
                        vymaxtext.setColor(checkedclr);
                        textbox=vymaxbox;
                        vymax=atof(vymaxtext.getString().toAnsiString().c_str());
                    }else
                    if((changingsettings)&&(event.mouseButton.x>=aytext.getPosition().x-8)&&(event.mouseButton.x<=aytext.getPosition().x-8+inputbars.getLocalBounds().width)&&(event.mouseButton.y>=aytext.getPosition().y-8)&&(event.mouseButton.y<=aytext.getPosition().y-8+inputbars.getLocalBounds().height)){
                        aytext.setColor(checkedclr);
                        textbox=aybox;
                        ay=atof(aytext.getString().toAnsiString().c_str());
                    }else
                    if((changingsettings)&&(event.mouseButton.x>=vjumptext.getPosition().x-8)&&(event.mouseButton.x<=vjumptext.getPosition().x-8+inputbars.getLocalBounds().width)&&(event.mouseButton.y>=vjumptext.getPosition().y-8)&&(event.mouseButton.y<=vjumptext.getPosition().y-8+inputbars.getLocalBounds().height)){
                        vjumptext.setColor(checkedclr);
                        textbox=vjumpbox;
                        vjump=atof(vjumptext.getString().toAnsiString().c_str());
                    }else
                    if((event.mouseButton.x>=maskrs.getPosition().x)&&(event.mouseButton.x<=maskrs.getPosition().x+maskrs.getLocalBounds().width)&&(event.mouseButton.y>=maskrs.getPosition().y)&&(event.mouseButton.y<=maskrs.getPosition().y+maskrs.getLocalBounds().height)){
                        choosedmask=(choosedmask+1)%10;
                        maskchoose.setTexture(maskchooset[choosedmask], 1);
                    }else
                    if((event.mouseButton.x>=maskls.getPosition().x)&&(event.mouseButton.x<=maskls.getPosition().x+maskls.getLocalBounds().width)&&(event.mouseButton.y>=maskls.getPosition().y)&&(event.mouseButton.y<=maskls.getPosition().y+maskls.getLocalBounds().height)){
                        if(!choosedmask) choosedmask=9;
                        else choosedmask--;
                        maskchoose.setTexture(maskchooset[choosedmask], 1);
                    }else
                    if((event.mouseButton.x>=okrgbm.getPosition().x)&&(event.mouseButton.x<=okrgbm.getPosition().x+okrgbm.getLocalBounds().width)&&(event.mouseButton.y>=okrgbm.getPosition().y)&&(event.mouseButton.y<=okrgbm.getPosition().y+okrgbm.getLocalBounds().height)){
                        protocol2a(yourcolordisplay.getFillColor(), choosedmask);
                    }
                    for(int i=0; i<3; i++)
                        if((event.mouseButton.x>=colorpointers[i].getPosition().x)&&(event.mouseButton.x<=colorpointers[i].getPosition().x+colorpointers[i].getLocalBounds().width)&&(event.mouseButton.y>=colorpointers[i].getPosition().y)&&(event.mouseButton.y<=colorpointers[i].getPosition().y+colorpointers[i].getLocalBounds().height)){
                            colorpointerpressed=i;
                        }
                }else
                if(event.type==sf::Event::TextEntered){sf::Text* inputpointer=0;
                    if(textbox==seedbox){
                        if(event.text.unicode==13){
                            seedinput.setColor(normalclr);
                            inputpointer=0;
                        }else
                        if((event.text.unicode>='0')&&(event.text.unicode<='9')&&(seedinput.getString().getSize()<10)){
                            seedinput.setString(seedinput.getString()+event.text.unicode);
                        }else
                        if(event.text.unicode==8) inputpointer=&seedinput;
                    }else
                    if(textbox==vxmaxbox){
                        if(event.text.unicode==13){
                            vxmaxtext.setColor(normalclr);
                            inputpointer=0;
                            vxmax=atof(vxmaxtext.getString().toAnsiString().c_str());
                        }else
                        if((event.text.unicode>='0')&&(event.text.unicode<='9')){
                            vxmaxtext.setString(vxmaxtext.getString()+event.text.unicode);
                        }else
                        if(event.text.unicode==8) inputpointer=&vxmaxtext;
                    }else
                    if(textbox==vymaxbox){
                        if(event.text.unicode==13){
                            vymaxtext.setColor(normalclr);
                            inputpointer=0;
                            vymax=atof(vymaxtext.getString().toAnsiString().c_str());
                        }else
                        if((event.text.unicode>='0')&&(event.text.unicode<='9')){
                            vymaxtext.setString(vymaxtext.getString()+event.text.unicode);
                        }else
                        if(event.text.unicode==8) inputpointer=&vymaxtext;
                    }else
                    if(textbox==aybox){
                        if(event.text.unicode==13){
                            aytext.setColor(normalclr);
                            inputpointer=0;
                            ay=atof(aytext.getString().toAnsiString().c_str());
                        }else
                        if((event.text.unicode>='0')&&(event.text.unicode<='9')){
                            aytext.setString(aytext.getString()+event.text.unicode);
                        }else
                        if(event.text.unicode==8) inputpointer=&aytext;
                    }else
                    if(textbox==vjumpbox){
                        if(event.text.unicode==13){
                            vjumptext.setColor(normalclr);
                            inputpointer=0;
                            vjump=atof(vjumptext.getString().toAnsiString().c_str());
                        }else
                        if((event.text.unicode>='0')&&(event.text.unicode<='9')){
                            vjumptext.setString(vjumptext.getString()+event.text.unicode);
                        }else
                        if(event.text.unicode==8){
                            buffer=vjumptext.getString();
                            if(buffer.length()-1){
                                buffer.erase(buffer.length()-1);
                                vjumptext.setString(buffer);
                            }
                        }
                    }

                    if(inputpointer){
                        if(event.text.unicode==8){
                            buffer=(*inputpointer).getString();
                            if(buffer.length()){
                                buffer.erase(buffer.length()-1);
                                (*inputpointer).setString(buffer);
                            }
                        }else
                        if((event.text.unicode==127)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            (*inputpointer).setString("");
                        }else
                        if((event.text.unicode==3)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            copyToClipboard((*inputpointer).getString());
                        }else
                        if((event.text.unicode==22)&&((sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))||(sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)))){
                            if(OpenClipboard(0)){
                                (*inputpointer).setString((*inputpointer).getString()+(char*)(GetClipboardData(CF_TEXT)));
                                CloseClipboard();
                            }
                        }else
                        {
                            (*inputpointer).setString((*inputpointer).getString()+event.text.unicode);
                        }
                    }
                }else
                if(event.type==sf::Event::MouseButtonReleased){
                    soundpointerpressed=0;
                    colorpointerpressed=3;
                }else
                if(event.type==sf::Event::MouseMoved){
                    if(soundpointerpressed){
                        soundtrack.setVolume((event.mouseMove.y-30)*(event.mouseMove.y>30)-(event.mouseMove.y-130)*(event.mouseMove.y>130));
                        soundpointers.setPosition(1139, 24+soundtrack.getVolume());
                    }else
                    if(colorpointerpressed<3){
                        colorpointers[colorpointerpressed].setPosition(((event.mouseMove.x)+((event.mouseMove.x<796)*(796-event.mouseMove.x))-((event.mouseMove.x>896)*(event.mouseMove.x-896))), colorpointers[colorpointerpressed].getPosition().y);
                        switch(colorpointerpressed){
                            case 0:{
                                yourcolor.r=(colorpointers[0].getPosition().x-796)*255/100;
                            }break;
                            case 1:{
                                yourcolor.g=(colorpointers[1].getPosition().x-796)*255/100;
                            }break;
                            case 2:{
                                yourcolor.b=(colorpointers[2].getPosition().x-796)*255/100;
                            }break;
                        }
                        yourcolordisplay.setFillColor(yourcolor);
                    }
                }
            }
        }
        if(bounds[3]){
            if(backgrounds.getPosition().x<-9){
                backgrounds.move(10, 0);
                deltabg.x+=10/mapscale;
                for(int i=0; i<wormpointers.size(); i++){
                    (*wormpointers[i]).update();
                }
            }else
            if(backgrounds.getPosition().x<0){
                backgrounds.setPosition(0, backgrounds.getPosition().y);
                deltabg.x=0;
                for(int i=0; i<wormpointers.size(); i++){
                    (*wormpointers[i]).update();
                }
            }
        }else
        if(bounds[1]){
            if(backgrounds.getPosition().x+(backgrounds.getLocalBounds().width*backgrounds.getScale().x)>window.getSize().x+9){
                backgrounds.move(-10, 0);
                deltabg.x-=10/mapscale;
                for(int i=0; i<wormpointers.size(); i++){
                    (*wormpointers[i]).update();
                }
            }else
            if(backgrounds.getPosition().x+(backgrounds.getLocalBounds().width*backgrounds.getScale().x)>window.getSize().x){
                backgrounds.setPosition(window.getSize().x-(backgrounds.getLocalBounds().width*backgrounds.getScale().x), backgrounds.getPosition().y);
                deltabg.x-=window.getSize().x-(backgrounds.getLocalBounds().width*backgrounds.getScale().x)/mapscale;
                for(int i=0; i<wormpointers.size(); i++){
                    (*wormpointers[i]).update();
                }
            }
        }
        if(bounds[0]){
            if(backgrounds.getPosition().y<-9){
                backgrounds.move(0, 10);
                deltabg.y+=10/mapscale;
                for(int i=0; i<wormpointers.size(); i++){
                    (*wormpointers[i]).update();
                }
            }else
            if(backgrounds.getPosition().y<0){
                backgrounds.setPosition(0, backgrounds.getPosition().y);
                deltabg.y=0;
                for(int i=0; i<wormpointers.size(); i++){
                    (*wormpointers[i]).update();
                }
            }
        }else
        if(bounds[2]){
            if(backgrounds.getPosition().y+(backgrounds.getLocalBounds().height*backgrounds.getScale().y)>window.getSize().y+9){
                backgrounds.move(0, -10);
                deltabg.y-=10/mapscale;
                for(int i=0; i<wormpointers.size(); i++){
                    (*wormpointers[i]).update();
                }
            }else
            if(backgrounds.getPosition().y+(backgrounds.getLocalBounds().height*backgrounds.getScale().y)>window.getSize().y){
                backgrounds.setPosition(window.getSize().y-(backgrounds.getLocalBounds().height*backgrounds.getScale().y), backgrounds.getPosition().y);
                deltabg.y-=window.getSize().y-(backgrounds.getLocalBounds().height*backgrounds.getScale().y)/mapscale;
                for(int i=0; i<wormpointers.size(); i++){
                    (*wormpointers[i]).update();
                }
            }
        }


        if(clientsocket.receive(data, 1024, received)==sf::Socket::Done){
            if(!receiving)
            for(int i=0; i<received; i++){
                if(!to_ignore){
                    if(!data[i]){
                        clientsocket.disconnect();
                        cout<<"\ndisconnected\n";
                        connectionS.setTexture(offt);
                        connected=0;
                        break;
                    }
                    if(data[i]==2){
                        i++;
                        if(i<received){
                            if(data[i]){
                                nickname=nickinput.getString();
                                cout<<"nick accepted\n"<<char(7);
                            }else{
                                cout<<"nick denied\n";
                            }
                        }else cout<<"lost response 0x2\n";
                        continue;
                    }
                    if(data[i]==5){
                        receiving=5;
                        deltareceive=i+1;
                        break;
                    }
                    if(data[i]==0x11){
                        receiving=0x11;
                        deltareceive=i+1;
                        break;
                    }
                    if(data[i]==0x21){
                        i++;
                        if(i<received){
                            if(data[i]){
                                mode=lobby;
                                lobbyname=roomnameinput.getString();
                                lobbynamet.setString(lobbyname);
                                changingsettings=1;
                                cout<<"room accepted\n"<<char(7);
                            }else{
                                cout<<"room denied\n";
                            }
                        }else cout<<"lost response 0x21\n";
                        continue;
                    }
                    if(data[i]==0x23){
                        i++;
                        if(i<received){
                            if(data[i]){
                                cout<<"physic accepted\n"<<char(7);
                                protocol24(atoi(seedinput.getString().toAnsiString().c_str()), playersamount);
                            }else{
                                cout<<"physic denied\n";
                            }
                        }else cout<<"lost response 0x23\n";
                        continue;
                    }
                    if(data[i]==0x25){
                        i++;
                        if(i<received){
                            if(data[i]){
                                mode=lobby;
                                lobbyname=roomnameinput.getString();
                                lobbynamet.setString(lobbyname);
                                changingsettings=0;
                                cout<<"settings accepted\n"<<char(7);
                            }else{
                                cout<<"settings denied\n";
                            }
                        }else cout<<"lost response 0xa\n";
                        continue;
                    }
                    if(data[i]==0x27){
                        receiving=0x27;
                        deltareceive=i+1;
                        to_receive=5;
                        protbufferi[2]=0;
                        passwordinput.setString(protbuffers[0]);
                        for(list<gamelistelements>::iterator i=gamelist.begin(); i!=gamelist.end(); ++i){
                            if(protbufferi[0]==(*i).id){
                                lobbyname=(*i).name;
                                lobbynamet.setString(lobbyname);
                                break;
                            }
                        }
                        break;
                    }
                    if(data[i]==0x29){cout<<"0x29\n";
                        receiving=0x29;
                        deltareceive=i+1;
                        to_receive=13;
                        protbufferi[0]=protbufferi[1]=protbufferi[2]=0;
                        break;
                    }
                    if(data[i]==0x2b){
                        i++;
                        if(i<received){
                            if(data[i]){
                                cout<<"player settings accepted\n"<<char(7);
                            }else{
                                cout<<"player settings denied\n";
                            }
                        }else cout<<"lost response 0x2b\n";
                        continue;
                    }
                    if(data[i]==0x2d){
                        i++;
                        if(i<received){
                            if(data[i]){
                                if(ready){
                                    ready=0;
                                    readys.setTexture(ready2);
                                    cout<<"unready\n"<<char(7);
                                }else{
                                    ready=1;
                                    readys.setTexture(ready1);
                                    cout<<"ready\n"<<char(7);
                                }
                            }else cout<<"changing ready denied\n";
                        }else cout<<"lost response 0x2d\n";
                        continue;
                    }
                    if(data[i]==0x30){
                        soundtrack.stop();
                        protocol2e();
                        backgroundt.loadFromImage(backgroundi=loadMap(seedinput.getString()+".map", spawnpoints));
                        backgrounds.setTexture(backgroundt, 1);
                        backgrounds.setScale(0.2,0.2);
                        mode=ingame;
                        protocol31();
                        break;
                    }
                    if(data[i]==0x2f){
                        receiving=0x2f;
                        deltareceive=i+1;
                        to_receive=5;
                        protbufferi[0]=1;
                        cout<<"receiving players\n";
                        break;
                    }
                    if(data[i]==0x33){
                        cout<<"your turn\n";
                        i++;
                        if(i<received){
                            if(data[i]<wormpointers.size()){
                                currentworm=wormpointers[data[i]];
                            }else cout<<"wrong worm id: "+to_string(int(data[i]))+"\n";
                        }else cout<<"lost response 0x19\n";
                        continue;
                    }
                    if(data[i]==0x35){
                        i++;//        time left
                        if(i<received){
                            turntime=data[i];
                            if(!turntime){
                                clocks.setTexture(clockt[0]);
                                clockframe=0;
                            }
                            turntimet.setString(to_string(turntime)+"s");
                        }else cout<<"lost response 0x35\n";
                        continue;
                    }
                    if(data[i]==0x36){
                        currentworm=0;
                        cout<<"end of your turn\n";
                        continue;
                    }
                    if(data[i]==0xe0){
                        cout<<"unknown opcode\n";
                        receiving=0xe0;
                        i++;
                        if(i<received){
                            to_receive=data[i];
                        }else{
                            cout<<"lost rest of 0xe0\n";
                            break;
                        }
                        deltareceive=i+1;
                        protbuffers[0]="";
                        break;
                    }
                    if(data[i]==0xe1){
                        cout<<"bad parameters\n";
                        receiving=0xe1;
                        i++;
                        if(i<received){
                            to_receive=data[i];
                        }else{
                            cout<<"lost rest of 0xe1\n";
                            break;
                        }
                        deltareceive=i+1;
                        protbuffers[0]="";
                        break;
                    }
                    if(data[i]==0xe2){
                        cout<<"server error:\n";
                        receiving=0xe2;
                        i++;
                        if(i<received){
                            to_receive=data[i];
                        }else{
                            cout<<"lost rest of 0xe2\n";
                            break;
                        }
                        deltareceive=i+1;
                        protbuffers[0]="";
                        break;
                    }
                    if(data[i]==0xe3){
                        cout<<"server error:\n";
                        receiving=0xe3;
                        i++;
                        if(i<received){
                            to_receive=data[i];
                        }else{
                            cout<<"lost rest of 0xe3\n";
                            break;
                        }
                        deltareceive=i+1;
                        protbuffers[0]="";
                        break;
                    }
                    cout<<"received unknown protocol: "<<int(data[i])<<"\n";
                }else to_ignore--;
            }
            if(receiving==5){
                for(int i=deltareceive; i<deltareceive+4; i++){
                    if(i>=received){
                        cout<<deltareceive+4-i<<"/"<<4<<" of id missed\n";
                        receiving=0;
                        break;
                    }
                    protbufferi[0]=protbufferi[0]<<8;
                    protbufferi[0]+=data[i];
                }
                myid=protbufferi[0];
                protbufferi[0]=0;
                cout<<"your id="<<myid<<"\n";
                deltareceive=0;
                receiving=0;
                protocol6();
            }else
            if(receiving==0x11){
                if(deltareceive){
                    for(int i=deltareceive; i<deltareceive+4; i++){
                        if(i>=received){
                            cout<<deltareceive+4-i<<"/"<<4<<" of rooms` list checksum missed\n";
                            receiving=0;
                            break;
                        }
                        to_receive=to_receive<<8;
                        to_receive+=data[i];
                    }
                    to_receive*=24;
                    deltareceive+=4;
                }
                if(to_receive==0){
                    cout<<"roomlist is empty\n";
                    receiving=0;
                    deltareceive=0;
                }else
                    for(int i=deltareceive; i<received; i++){
                        if(to_receive%24==0){
                            lastgamelistelement++;
                            if(!gamelist.empty()){
                                list<gamelistelements>::iterator j=gamelist.end(); j--;
                                (*j).update();
                            }
                            gamelist.push_back(gamelistelements(lastgamelistelement));
                        }
                        to_receive--;
                        list<gamelistelements>::iterator j=gamelist.end(); j--;
                        if(to_receive%24>19){
                            (*j).id=(*j).id<<8;
                            (*j).id+=data[i];
                        }else{
                            if(data[i])
                                (*j).name+=data[i];
                        }
                        if(!to_receive){
                            (*j).update();
                            receiving=0;
                            deltareceive=0;
                            break;
                        }
                    }
                deltareceive=0;
            }else
            if(receiving==0x27){
                for(int i=deltareceive; ((i<received)||(to_receive>0)); i++){
                    if(to_receive==5){
                        if(data[i]){
                            cout<<"joined\n"<<char(7);
                            mode=lobby;
                            protocol28();
                        }else{
                            cout<<"not joined\n";
                            to_receive=0;
                            break;
                        }
                    }
                    else{
                        protbufferi[2]=protbufferi[2]<<8;
                        protbufferi[2]+=data[i];
                    }

                    to_receive--;
                }
                deltareceive=0;
                if(!to_receive){
                    seedinput.setString(to_string(protbufferi[2]));
                    receiving=0;
                }
            }else
            if(receiving==0x29){
                for(int i=deltareceive; (i<received); i++){
                    if(to_receive>9){
                        protbufferi[0]=protbufferi[0]<<8;
                        protbufferi[0]+=data[i];
                    }else
                    if(to_receive>7){
                        protbufferi[1]=protbufferi[1]<<8;
                        protbufferi[1]+=data[i];
                    }else
                    if(to_receive>5){
                        protbufferi[2]=protbufferi[2]<<8;
                        protbufferi[2]+=data[i];
                    }else
                    if(to_receive>3){
                        protbufferi[3]=protbufferi[3]<<8;
                        protbufferi[3]+=data[i];
                    }else
                    if(to_receive>1){
                        protbufferi[4]=protbufferi[4]<<8;
                        protbufferi[4]+=data[i];
                    }else
                    if(to_receive==1){
                        seedinput.setString(to_string(protbufferi[0]));
                        ay=protbufferi[1];
                        vjump=-protbufferi[2];
                        vymax=protbufferi[3];
                        vxmax=protbufferi[4];
                        cb2players.setTexture(checkboxoff);
                        cb3players.setTexture(checkboxoff);
                        cb4players.setTexture(checkboxoff);
                        switch(data[i]){
                            case 2:{
                                cb2players.setTexture(checkboxon);
                            }break;
                            case 3:{
                                cb3players.setTexture(checkboxon);
                            }break;
                            case 4:{
                                cb4players.setTexture(checkboxon);
                            }break;
                            default:{
                                cout<<"wrong required players: "<<data[i]<<"\n";
                            }break;
                        }
                        receiving=0;
                    }
                    to_receive--;
                }
            }else
            if(receiving==0x2f){
                for(int i=deltareceive; i<received; i++){
                    if(protbufferi[0]){
                        if(to_receive==5){
                            //data[i]==gotowi
                        }else{
                            protbufferi[1]=protbufferi[1]<<8;
                            protbufferi[1]+=data[i];
                        }
                    }else{
                        if(!(to_receive%25)){
                            if(protbufferi[1]!=40){players.push_back(player(protbufferi[1], protbuffers[0], sf::Color(protbufferi[2], protbufferi[3], protbufferi[4]), protbufferi[5]));
                            }
                            protbufferi[1]=data[i];
                            protbuffers[0]="";
                        }else
                        if(to_receive%25>4){
                            if(data[i])
                                protbuffers[0]+=data[i];
                        }else
                        if(to_receive%25>3){
                            if(data[i])
                                protbufferi[2]=data[i];
                        }else
                        if(to_receive%25>2){
                            if(data[i])
                                protbufferi[3]=data[i];
                        }else
                        if(to_receive%25>1){
                            if(data[i])
                                protbufferi[4]=data[i];
                        }else
                        if(to_receive%25>0){
                            if(data[i])
                                protbufferi[5]=data[i];
                        }
                    }
                    to_receive--;
                    if(!to_receive){
                        if(protbufferi[0]){//end of metadata
                            players.clear();
                            protbufferi[0]=0;
                            if(protbufferi[1]==0){
                                cout<<"empty player list?\ndisconnecting\n";
                                receiving=0;
                                clientsocket.disconnect();
                                break;
                            }
                            to_receive=25*protbufferi[1];
                            protbufferi[1]=40;
                        }else
                        {//end of protocol
                            receiving=0;
                            players.push_back(player(protbufferi[1], protbuffers[0], sf::Color(protbufferi[2], protbufferi[3], protbufferi[4]), protbufferi[5]));
                            protbufferi[1]=0;
                            protbuffers[0]="";
                            for(int i=0; i<players.size(); i++){
                                cout<<"player["<<i<<"]="<<int(players[i].id)<<", "<<players[i].name<<"\n";
                            }
                            break;
                        }
                    }
                }
            }else
            if(receiving==0xe0){
                for(int i=deltareceive; i<received; i++){
                    to_receive--;
                    if(!to_receive){
                        protbuffers[0]+=data[i];
                        cout<<protbuffers[0]<<"\n";
                        MessageBox(0, protbuffers[0].c_str(), "0xe0: unknown opcode", MB_OK);
                        receiving=0;
                        break;
                    }
                    protbuffers[0]+=data[i];
                }
            }else
            if(receiving==0xe1){
                for(int i=deltareceive; i<received; i++){
                    to_receive--;
                    if(!to_receive){
                        protbuffers[0]+=data[i];
                        cout<<protbuffers[0]<<"\n";
                        MessageBox(0, protbuffers[0].c_str(), "0xe1: unknown opcode", MB_OK);
                        receiving=0;
                        break;
                    }
                    protbuffers[0]+=data[i];
                }
            }else
            if(receiving==0xe2){
                for(int i=deltareceive; i<received; i++){
                    to_receive--;
                    if(!to_receive){
                        protbuffers[0]+=data[i];
                        cout<<protbuffers[0]<<"\n";
                        MessageBox(0, protbuffers[0].c_str(), "0xe2: server error", MB_OK);
                        receiving=0;
                        break;
                    }
                    protbuffers[0]+=data[i];
                }
            }else
            if(receiving==0xe3){
                for(int i=deltareceive; i<received; i++){
                    to_receive--;
                    if(!to_receive){
                        protbuffers[0]+=data[i];
                        cout<<protbuffers[0]<<"\n";
                        MessageBox(0, protbuffers[0].c_str(), "0xe3: server error", MB_OK);
                        receiving=0;
                        break;
                    }
                    protbuffers[0]+=data[i];
                }
            }else
            if(receiving){
                cout<<"wtf? unknown receiving\n";
                receiving=0;
            }
        }
        sf::IpAddress aelirsydgfbheljrkd; unsigned short arisdhbskufghkarudh;
        if(udpsocket.receive(data, 1024, received, aelirsydgfbheljrkd, arisdhbskufghkarudh)!=sf::Socket::Done){
            if(!receiving)
            for(int i=0; i<received; i++){
                if(!udpto_ignore){
                    if(data[i]==7){
                        cout<<"UDP connection established\n";
                        continue;
                    }
                    if(data[i]==0x32){
                        receiving=0x32;
                        udpdeltareceive=i+1;
                        udpto_receive=4;
                        protbufferi[0]=1;
                        for(int j=1; j<7; j++)
                            protbufferi[j]=0;
                        break;
                    }
                    cout<<"recieved unknown UDP protocol: "<<int(data[i])<<"\n";
                }else udpto_ignore--;
            }


            if(receiving==0x32){
                for(int i=udpdeltareceive; i<received; i++){//1=team, 2=x, 3=y, 4=hp, 5=id, 6=Vx, 7=Vy
                    if(protbufferi[0]){
                        protbufferi[1]=protbufferi[1]<<8;
                        protbufferi[1]+=data[i];
                    }else{
                        if(!(udpto_receive%15)){
                            if(protbufferi[1]<4){
                                if(players[protbufferi[1]].addworm(worm(sf::Vector2f(protbufferi[2], protbufferi[3]), protbufferi[1], protbufferi[4], protbufferi[5]))){
                                    worm *wpbuf=&players[protbufferi[1]].worms[players[protbufferi[1]].emptyworm-1];
                                    (*wpbuf).V=sf::Vector2f(protbufferi[6], protbufferi[7]);
                                    wormpointers.push_back(wpbuf);
                                }else{
                                    for(int j=0; j<wormpointers.size(); j++){
                                        if((*wormpointers[j]).id==protbufferi[5]){
                                            (*wormpointers[j])=worm(sf::Vector2f(protbufferi[2], protbufferi[3]), protbufferi[1], protbufferi[4], protbufferi[5]);
                                            (*wormpointers[j]).V=sf::Vector2f(protbufferi[6], protbufferi[7]);
                                            break;
                                        }
                                    }
                                }
                            }else{
                                if(protbufferi[1]!=40)
                                    cout<<"wrong data, playerid="<<protbufferi[1]<<"\n";
                            }
                            protbufferi[1]=data[i];
                        }else
                        if((to_receive%15)>=11){
                            protbufferi[2]=protbufferi[2]<<8;
                            protbufferi[2]+=data[i];
                        }else
                        if((to_receive%15)>=7){
                            protbufferi[3]=protbufferi[3]<<8;
                            protbufferi[3]+=data[i];
                        }else
                        if((to_receive%15)==6){
                            protbufferi[4]=data[i];
                        }else
                        if((to_receive%15)==5){
                            protbufferi[5]=data[i];
                        }else
                        if((to_receive%15)>=3){
                            protbufferi[6]=protbufferi[6]<<8;
                            protbufferi[6]+=data[i];
                        }else
                        if((to_receive%15)>=1){
                            protbufferi[7]=protbufferi[7]<<8;
                            protbufferi[7]+=data[i];
                        }
                    }
                    udpto_receive--;
                    if(!udpto_receive){
                        if(protbufferi[0]){//end of metadata
                            protbufferi[0]=0;
                            udpto_receive=11*protbufferi[1];
                            if(protbufferi[1]==0){
                                cout<<"empty worms list?\ndisconnecting\n";
                                receiving=0;
                                clientsocket.disconnect();
                                break;
                            }
                            protbufferi[1]=40;
                        }else
                        {//end of protocol
                            receiving=0;
                            if(protbufferi[1]<4){
                                if(players[protbufferi[1]].addworm(worm(sf::Vector2f(protbufferi[2], protbufferi[3]), protbufferi[1], protbufferi[4], protbufferi[5]))){
                                    worm *wpbuf=&players[protbufferi[1]].worms[players[protbufferi[1]].emptyworm-1];
                                    (*wpbuf).V=sf::Vector2f(protbufferi[6], protbufferi[7]);
                                    wormpointers.push_back(wpbuf);
                                }
                                else{
                                    for(int j=0; j<wormpointers.size(); j++){
                                        if((*wormpointers[j]).id==protbufferi[5]){
                                            (*wormpointers[j])=worm(sf::Vector2f(protbufferi[2], protbufferi[3]), protbufferi[1], protbufferi[4], protbufferi[5]);
                                            (*wormpointers[j]).V=sf::Vector2f(protbufferi[6], protbufferi[7]);
                                            break;
                                        }
                                    }
                                }
                            }else{
                                cout<<"wrong data, playerid="<<protbufferi[1]<<"\n";
                            }
                            for(int j=1; j<7; j++)
                                protbufferi[j]=0;
                            break;
                        }
                    }
                }
                no18delta=0;
            }
        }
        udpport=udpsocket.getLocalPort();
        serverip=clientsocket.getRemoteAddress();
        frame++;
        if(!(frame%120)){
            if(clientsocket.getRemoteAddress()==sf::IpAddress::None){
                if(connected){
                    connected=0;
                    connectionS.setTexture(offt);
                    cout<<"connection lost\n";
                }
            }else{
                if(!connected){
                    connected=1;
                    connectionS.setTexture(ont);
                    cout<<"reconnected\n";
                }
            }
        }
        if(!(frame%20)){
            if((turntime)&&(connected)){
                clockframe++;
                if(clockframe>7)
                    clockframe=0;
                clocks.setTexture(clockt[clockframe]);
            }
        }
        if(!(frame%60)){
            if((mode==lobby)&&(connected)){
                protocol2e();
            }
        }

        lastittime=currentittime;
        currentittime=clocker.getElapsedTime();
        if(started){
            if((!backgroundi.getSize().x)||(!backgroundi.getSize().y)){
                cout<<"fatal error, map invalid, closing game\n";
                system("pause");
                return 1;
            }
            for(int i=0; i<wormpointers.size(); i++){
                if((*wormpointers[i]).hp>0){
                    if((*wormpointers[i]).V.y==0){
                        int wuk=(*wormpointers[i]).position.y+(*wormpointers[i]).sprite.getLocalBounds().height;
                        bool fhasfloor=0;
                        for(int j=(*wormpointers[i]).position.x; j<(*wormpointers[i]).position.x+(*wormpointers[i]).sprite.getLocalBounds().width; j++){
                            if(colide(sf::Vector2f(j, wuk), backgroundi)){
                                fhasfloor=1;
                                break;
                            }
                        }
                        if(!fhasfloor){
                            (*wormpointers[i]).V.y+=ay*(currentittime-lastittime).asSeconds();
                            if((*wormpointers[i]).V.y>vymax)
                                (*wormpointers[i]).V.y=vymax;
                        }else
                        if((*wormpointers[i]).V.x>0){
                            int fkas=(*wormpointers[i]).position.x+(*wormpointers[i]).sprite.getLocalBounds().width;
                            bool fcolided=0;
                            sf::Vector2f colisionpos;
                            for(int k=0; k<=(currentittime-lastittime).asSeconds()*(*wormpointers[i]).V.x; k++){
                                for(int j=(*wormpointers[i]).position.y+(*wormpointers[i]).sprite.getLocalBounds().height-1; j>=(*wormpointers[i]).position.y; j--){
                                    if(colide(sf::Vector2f(k+fkas, j), backgroundi)){
                                        fcolided=1;
                                        colisionpos=sf::Vector2f(k+(*wormpointers[i]).position.x, (*wormpointers[i]).position.y);
                                        break;
                                    }
                                }
                                if(fcolided)
                                    break;
                            }
                            if(fcolided){
                                (*wormpointers[i]).position=colisionpos;
                                (*wormpointers[i]).update();
                            }else{
                                (*wormpointers[i]).position.x+=(currentittime-lastittime).asSeconds()*(*wormpointers[i]).V.x;
                                (*wormpointers[i]).update();
                            }
                            if(!(frame%5)){
                                (*wormpointers[i]).next_anim();
                            }
                        }else
                        if((*wormpointers[i]).V.x<0){
                            bool fcolided=0;
                            sf::Vector2f colisionpos;
                            for(int k=0; k>=(currentittime-lastittime).asSeconds()*(*wormpointers[i]).V.x; k--){
                                for(int j=(*wormpointers[i]).position.y+(*wormpointers[i]).sprite.getLocalBounds().height-1; j>=(*wormpointers[i]).position.y; j--){
                                    if(colide(sf::Vector2f(k+(*wormpointers[i]).position.x, j), backgroundi)){
                                        fcolided=1;
                                        colisionpos=sf::Vector2f(k+(*wormpointers[i]).position.x, (*wormpointers[i]).position.y);
                                        break;
                                    }
                                }
                                if(fcolided)
                                    break;
                            }
                            if(fcolided){
                                (*wormpointers[i]).position=colisionpos;
                                (*wormpointers[i]).update();
                            }else{
                                (*wormpointers[i]).position.x+=(currentittime-lastittime).asSeconds()*(*wormpointers[i]).V.x;
                                (*wormpointers[i]).update();
                            }
                            if(!(frame%5)){
                                (*wormpointers[i]).next_anim();
                            }
                        }
                    }
                    if((*wormpointers[i]).V.y>0){
                        if(!(*wormpointers[i]).V.x){
                            bool fcolided=0;
                            sf::Vector2f colisionpos;
                            for(int k=0; k<=(currentittime-lastittime).asSeconds()*(*wormpointers[i]).V.y; k++){
                                for(int j=(*wormpointers[i]).position.x; j<=(*wormpointers[i]).position.x+(*wormpointers[i]).sprite.getLocalBounds().width; j++){
                                    if(colide(sf::Vector2f(j, ((*wormpointers[i]).position.y+(*wormpointers[i]).sprite.getLocalBounds().height+k)), backgroundi)){
                                        fcolided=1;
                                        colisionpos=sf::Vector2f(j, ((*wormpointers[i]).position.y+k));
                                        break;
                                    }
                                }
                                if(fcolided)
                                    break;
                            }
                            if(fcolided){
                                (*wormpointers[i]).position=colisionpos;
                                (*wormpointers[i]).V.y=0;
                            }else{
                                (*wormpointers[i]).position.y+=int((currentittime-lastittime).asSeconds()*(*wormpointers[i]).V.y);
                                (*wormpointers[i]).V.y+=ay*(currentittime-lastittime).asSeconds();
                                if((*wormpointers[i]).V.y>vymax)
                                    (*wormpointers[i]).V.y=vymax;
                            }
                            (*wormpointers[i]).update();
                        }else
                        if((*wormpointers[i]).V.x>0){//  `.
                            bool fcolided=0, mdir;
                            sf::Vector2f colisionpos;
                            float maxVel=(*wormpointers[i]).V.x, minVel;
                            if(maxVel>(*wormpointers[i]).V.y){
                                minVel=(*wormpointers[i]).V.y;
                                mdir=0;
                            }else{
                                minVel=maxVel;
                                maxVel=(*wormpointers[i]).V.y;
                                mdir=1;
                            }
                            if(mdir){
                                for(int j=0; j<maxVel*(currentittime-lastittime).asSeconds(); j++){
                                    for(int k=0; k<wormt[0].getSize().x; k++){
                                        if(colide(sf::Vector2f(int(k+j/minVel)+(*wormpointers[i]).position.x, (*wormpointers[i]).position.y+wormt[0].getSize().y+j), backgroundi)){
                                            fcolided=1;
                                            colisionpos=sf::Vector2f(int(j/minVel)+(*wormpointers[i]).position.x, (*wormpointers[i]).position.y+j-1);
                                            break;
                                        }
                                        if((k==wormt[0].getSize().x-1)&&(int(j/minVel)>int((j-1)/minVel))){
                                            for(int l=0; l<wormt[0].getSize().y; l++){
                                                if(colide(sf::Vector2f(int(j/minVel)+(*wormpointers[i]).position.x+k, (*wormpointers[i]).position.y+l), backgroundi)){
                                                    fcolided=1;
                                                    colisionpos=sf::Vector2f(int(j/minVel)+(*wormpointers[i]).position.x, (*wormpointers[i]).position.y+j-1);
                                                    break;
                                                }
                                            }
                                            if(fcolided)
                                                break;
                                        }
                                    }
                                    if(fcolided)
                                        break;
                                }
                            }else{//--------------------------------------------------------------------------------------------------------------------------to do

                            }
                            if(fcolided){
                                (*wormpointers[i]).position=colisionpos;
                                (*wormpointers[i]).V.y=0;
                                if((*wormpointers[i]).V.x!=vxmax)(*wormpointers[i]).V.x=0;
                            }else{
                                (*wormpointers[i]).position.x+=int((*wormpointers[i]).V.x*(currentittime-lastittime).asSeconds());
                                (*wormpointers[i]).position.y+=int((*wormpointers[i]).V.y*(currentittime-lastittime).asSeconds());
                                (*wormpointers[i]).V.y+=ay*(currentittime-lastittime).asSeconds();
                                if((*wormpointers[i]).V.y>vymax)
                                    (*wormpointers[i]).V.y=vymax;
                            }
                            (*wormpointers[i]).update();
                        }
                    }else
                    if((*wormpointers[i]).V.y<0){
                        if(!(*wormpointers[i]).V.x){
                            bool fcolided=0;
                            sf::Vector2f colisionpos;
                            for(int k=0; k>=(currentittime-lastittime).asSeconds()*(*wormpointers[i]).V.y; k--){
                                for(int j=(*wormpointers[i]).position.x; j<=(*wormpointers[i]).position.x+(*wormpointers[i]).sprite.getLocalBounds().width; j++){
                                    if(colide(sf::Vector2f(j, ((*wormpointers[i]).position.y+k)), backgroundi)){
                                        fcolided=1;
                                        colisionpos=sf::Vector2f(j, ((*wormpointers[i]).position.y+k));
                                        break;
                                    }
                                }
                                if(fcolided)
                                    break;
                            }
                            if(fcolided){
                                (*wormpointers[i]).position=colisionpos;
                                (*wormpointers[i]).V.y=0;
                            }else{
                                (*wormpointers[i]).position.y+=int((currentittime-lastittime).asSeconds()*(*wormpointers[i]).V.y);
                            }
                            (*wormpointers[i]).V.y+=ay*(currentittime-lastittime).asSeconds();
                            if((*wormpointers[i]).V.y>vymax)
                                (*wormpointers[i]).V.y=vymax;
                            (*wormpointers[i]).update();
                        }
                    }
                }
            }
        }

        window.clear(bgcolor);{
        if(mode==ingame){
            window.draw(backgrounds);
            for(int i=0; i<wormpointers.size(); i++){
                (*wormpointers[i]).draw(window);
            }
            window.draw(clocks);
            window.draw(turntimet);
        }else
        if(mode==connectroom){
            if(connected)
                for(list<gamelistelements>::iterator i=gamelist.begin(); i!=gamelist.end(); ++i){
                    (*i).draw(window, inputbars, binputbars);
                }
            window.draw(bgplanes);
            inputbars.setPosition(ipinput.getPosition()+sf::Vector2f(-8,-8));
            window.draw(inputbars);
            window.draw(ipinput);
            inputbars.setPosition(portinput.getPosition()+sf::Vector2f(-8,-8));
            window.draw(inputbars);
            window.draw(portinput);
            binputbars.setPosition(nickinput.getPosition()+sf::Vector2f(-8,-8));
            window.draw(binputbars);
            window.draw(nickinput);
            binputbars.setPosition(roomnameinput.getPosition()+sf::Vector2f(-8,-8));
            window.draw(binputbars);
            window.draw(roomnameinput);
            binputbars.setPosition(passwordinput.getPosition()+sf::Vector2f(-8,-8));
            window.draw(binputbars);
            window.draw(passwordinput);
            for(int i=0; i<5; i++)
                window.draw(info[i]);
            if(connected)
                for(int i=5; i<7; i++)
                    window.draw(info[i]);
            window.draw(okconnects);
            window.draw(oknicks);
            window.draw(okcreaterooms);
            window.draw(reloadgamelists);
            window.draw(connectionS);
            window.draw(soundicons);
            if(soundbarexchanged){
                window.draw(soundbars);
                window.draw(soundpointers);
            }
        }else
        if(mode==lobby){
            window.draw(maskrs);
            window.draw(maskls);
            window.draw(maskchoose);
            window.draw(yourcolordisplay);
            window.draw(okrgbm);
            for(int i=0; i<3; i++){
                window.draw(colorbars[i]);
                window.draw(colorpointers[i]);
            }
            window.draw(connectionS);
            window.draw(lobbynamet);
            window.draw(lobbyouts);
            inputbars.setPosition(seedinput.getPosition()+sf::Vector2f(-8,-8));
            window.draw(inputbars);
            window.draw(seedinput);
            window.draw(info[7]);
            if(changingsettings){
                for(int i=8; i<INFO_AMOUNT; i++)
                    window.draw(info[i]);
                window.draw(cb2players);
                window.draw(cb3players);
                window.draw(cb4players);
                window.draw(oksettings);
            }
            window.draw(readys);
            window.draw(advanceds);
            if(advancedb){
                inputbars.setPosition(vxmaxtext.getPosition()+sf::Vector2f(-8,-8));
                window.draw(inputbars);
                window.draw(vxmaxtext);
                inputbars.setPosition(vymaxtext.getPosition()+sf::Vector2f(-8,-8));
                window.draw(inputbars);
                window.draw(vymaxtext);
                inputbars.setPosition(aytext.getPosition()+sf::Vector2f(-8,-8));
                window.draw(inputbars);
                window.draw(aytext);
                inputbars.setPosition(vjumptext.getPosition()+sf::Vector2f(-8,-8));
                window.draw(inputbars);
                window.draw(vjumptext);
                for(int i=0; i<4; i++){
                    window.draw(ainfo[i]);
                }
            }
            window.draw(soundicons);
            if(soundbarexchanged){
                window.draw(soundbars);
                window.draw(soundpointers);
            }
        }
        }window.display();
    }
    return 0;
}

//"Mo¿liwe, ¿e bêdzie dzia³aæ, ale prawdopodobnie nie." Micha³ Marczewski
//"Obawiam się, że będziemy się na spawnie spawnić" Jakub Olszewski
