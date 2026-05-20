#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>

static const int   CELL  = 20;
static const int   COLS  = 28;
static const int   ROWS  = 31;
static const int   WIN_W = COLS * CELL;
static const int   WIN_H = ROWS * CELL + 60;
static const float PI    = 3.14159265f;

enum Dir { D_NONE=-1, D_R=0, D_U=1, D_L=2, D_D=3 };
enum GameState { MENU, PLAYING, PAUSED, GAME_OVER, WIN, HELP };

static int DC[4] = {1,0,-1,0};
static int DR[4] = {0,-1,0,1};

static Dir opposite(Dir d){
    switch(d){case D_R:return D_L;case D_L:return D_R;
              case D_U:return D_D;case D_D:return D_U;default:return D_NONE;}
}

static const int MT[ROWS][COLS] = {
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
{1,3,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,3,1},
{1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1},
{1,0,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1},
{1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1},
{1,1,1,1,1,1,0,1,1,1,1,1,2,1,1,2,1,1,1,1,1,0,1,1,1,1,1,1},
{1,1,1,1,1,1,0,1,1,1,1,1,2,1,1,2,1,1,1,1,1,0,1,1,1,1,1,1},
{1,1,1,1,1,1,0,1,1,2,2,2,2,2,2,2,2,2,2,1,1,0,1,1,1,1,1,1},
{1,1,1,1,1,1,0,1,1,2,1,1,1,4,4,1,1,1,2,1,1,0,1,1,1,1,1,1},
{1,1,1,1,1,1,0,1,1,2,1,2,2,2,2,2,2,1,2,1,1,0,1,1,1,1,1,1},
{2,2,2,2,2,2,0,2,2,2,1,2,2,2,2,2,2,1,2,2,2,0,2,2,2,2,2,2},
{1,1,1,1,1,1,0,1,1,2,1,2,2,2,2,2,2,1,2,1,1,0,1,1,1,1,1,1},
{1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1},
{1,1,1,1,1,1,0,1,1,2,2,2,2,2,2,2,2,2,2,1,1,0,1,1,1,1,1,1},
{1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1},
{1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
{1,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
{1,3,0,0,1,1,0,0,0,0,0,0,0,2,2,0,0,0,0,0,0,0,1,1,0,0,3,1},
{1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,1,1},
{1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,0,1,1,0,1,1,0,1,1,1},
{1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1},
{1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
{1,0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,0,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
static int maze[ROWS][COLS];
static bool dots[ROWS][COLS];
static bool pdots[ROWS][COLS];

struct Pt { float x,y,vx,vy,life,r,g,b; };

struct Ghost {
    int col,row,tc,tr;
    float progress,speed,x,y,r,g,b;
    int id;
    bool frightened; float frighTimer;
    bool eaten,inHouse; float houseWait;
    Dir lastDir;
    std::vector<Pt> fire; float fireT;
};

struct Pacman {
    float x,y;
    int col,row,tc,tr;
    float progress,speed;
    Dir curDir,nextDir;
    float mouthAngle; int mouthDir;
    int lives,score;
    bool dying; float dyingTimer;
};

static Pacman pac;
static std::vector<Ghost> ghosts;
static GameState gs = MENU;
static int menuSel  = 0;
static int hiScore  = 0;
static int level    = 1;
static float gameTime = 0;
static int dotsLeft = 0;

// ── Maze helpers ─────────────────────────────────────────────
static bool isWall(int c,int r){ if(c<0||c>=COLS||r<0||r>=ROWS) return true; return maze[r][c]==1; }
static bool isDoor(int c,int r){ if(c<0||c>=COLS||r<0||r>=ROWS) return false; return maze[r][c]==4; }
static bool gOK(int c,int r,bool ad=false){
    if(c<0||c>=COLS||r<0||r>=ROWS) return false;
    if(maze[r][c]==1) return false;
    if(isDoor(c,r)&&!ad) return false;
    return true;
}
static bool pOK(int c,int r){
    if(r<0||r>=ROWS) return false;
    if(c<0)c=COLS-1; if(c>=COLS)c=0;
    return maze[r][c]!=1&&!isDoor(c,r);
}

// ── Ghost pathfinding ─────────────────────────────────────────
static void pickNext(Ghost& g,float tx,float ty,bool ad){
    int gc=g.tc,gr=g.tr;
    Dir noGo=opposite(g.lastDir);
    int bC=gc,bR=gr; float bD=1e18f; bool found=false;
    for(int d=0;d<4;d++){
        if((Dir)d==noGo) continue;
        int nc=gc+DC[d],nr=gr+DR[d];
        if(!gOK(nc,nr,ad)) continue;
        float dx=(nc+0.5f)*CELL-tx,dy=(nr+0.5f)*CELL-ty,dd=dx*dx+dy*dy;
        if(!found||dd<bD){bD=dd;bC=nc;bR=nr;found=true;}
    }
    if(!found){ Dir rv=opposite(g.lastDir); if(rv!=D_NONE){int nc=gc+DC[(int)rv],nr=gr+DR[(int)rv];if(gOK(nc,nr,ad)){bC=nc;bR=nr;found=true;}} }
    if(!found){ for(int d=0;d<4;d++){int nc=gc+DC[d],nr=gr+DR[d];if(gOK(nc,nr,ad)){bC=nc;bR=nr;break;}} }
    int ddc=bC-gc,ddr=bR-gr; g.lastDir=D_NONE;
    for(int d=0;d<4;d++) if(DC[d]==ddc&&DR[d]==ddr){g.lastDir=(Dir)d;break;}
    g.col=gc;g.row=gr;g.tc=bC;g.tr=bR;g.progress=0;
}
static void pickRand(Ghost& g,bool ad){
    int gc=g.tc,gr=g.tr; Dir noGo=opposite(g.lastDir);
    int valid[4],nv=0;
    for(int d=0;d<4;d++){if((Dir)d==noGo)continue;int nc=gc+DC[d],nr=gr+DR[d];if(gOK(nc,nr,ad))valid[nv++]=d;}
    int ch=-1;
    if(nv>0)ch=valid[rand()%nv];
    else for(int d=0;d<4;d++){int nc=gc+DC[d],nr=gr+DR[d];if(gOK(nc,nr,ad)){ch=d;break;}}
    if(ch<0){g.col=gc;g.row=gr;g.tc=gc;g.tr=gr;g.progress=0;return;}
    g.lastDir=(Dir)ch;g.col=gc;g.row=gr;g.tc=gc+DC[ch];g.tr=gr+DR[ch];g.progress=0;
}
static void ghostTarget(const Ghost& g,float& tx,float& ty){
    if(g.id==0){tx=pac.x;ty=pac.y;return;}
    if(g.id==1){int d=(pac.curDir==D_NONE?D_R:pac.curDir);tx=pac.x+DC[d]*4*CELL;ty=pac.y+DR[d]*4*CELL;return;}
    if(g.id==2&&!ghosts.empty()){int d=(pac.curDir==D_NONE?D_R:pac.curDir);float p2x=pac.x+DC[d]*2*CELL,p2y=pac.y+DR[d]*2*CELL;tx=2*p2x-ghosts[0].x;ty=2*p2y-ghosts[0].y;return;}
    if(g.id==3){float d2=(g.x-pac.x)*(g.x-pac.x)+(g.y-pac.y)*(g.y-pac.y);if(d2>(8*CELL)*(8*CELL)){tx=pac.x;ty=pac.y;}else{tx=CELL;ty=29*CELL;}return;}
    tx=pac.x;ty=pac.y;
}
static void updateFire(Ghost& g,float dt){
    if(g.id!=0||g.inHouse||g.eaten||g.frightened)return;
    g.fireT-=dt;
    if(g.fireT<=0){
        g.fireT=1.5f+(float)(rand()%3);
        float px=((1-g.progress)*g.col+g.progress*g.tc+0.5f)*CELL;
        float py=((1-g.progress)*g.row+g.progress*g.tr+0.5f)*CELL;
        for(int i=0;i<10;i++){Pt p;p.x=px;p.y=py;float a=rand()%360*PI/180,sp=12+(rand()%20);p.vx=cosf(a)*sp;p.vy=sinf(a)*sp;p.life=1;p.r=1;p.g=0.1f+(rand()%6)*0.1f;p.b=0;g.fire.push_back(p);}
    }
    for(auto& p:g.fire){p.x+=p.vx*dt;p.y+=p.vy*dt;p.life-=dt*1.3f;}
    g.fire.erase(std::remove_if(g.fire.begin(),g.fire.end(),[](const Pt& p){return p.life<=0;}),g.fire.end());
}
static void stepGhost(Ghost& g,float dt){
    float tileTime=1.0f/g.speed;
    g.progress+=dt/tileTime;
    if(g.progress>=1.0f){
        g.progress=1.0f;g.x=(g.tc+0.5f)*CELL;g.y=(g.tr+0.5f)*CELL;
        bool ad=g.eaten;
        if(g.frightened)pickRand(g,ad);
        else{float tx,ty;ghostTarget(g,tx,ty);pickNext(g,tx,ty,ad);}
    }
}
static Ghost makeGhost(int id){
    Ghost g;g.id=id;g.progress=0;g.frightened=false;g.frighTimer=0;g.eaten=false;g.inHouse=true;g.fireT=2;g.fire.clear();g.lastDir=D_L;g.speed=3.0f+(level-1)*0.2f;
    static const int SC[4]={13,13,11,15},SR[4]={14,13,13,13};
    g.col=SC[id];g.row=SR[id];g.tc=SC[id];g.tr=SR[id];g.houseWait=(float)id*3.0f;
    switch(id){case 0:g.r=1;g.g=0.1f;g.b=0.1f;break;case 1:g.r=1;g.g=0.6f;g.b=0.8f;break;case 2:g.r=0;g.g=0.9f;g.b=0.9f;break;case 3:g.r=1;g.g=0.6f;g.b=0.1f;break;}
    g.x=(g.col+0.5f)*CELL;g.y=(g.row+0.5f)*CELL;return g;
}
static void initPac(){
    pac.col=13;pac.row=23;pac.tc=13;pac.tr=23;
    pac.x=(pac.col+0.5f)*CELL;pac.y=(pac.row+0.5f)*CELL;
    pac.progress=0;pac.speed=5.0f+(level-1)*0.3f;
    pac.curDir=D_L;pac.nextDir=D_L;
    pac.mouthAngle=25;pac.mouthDir=1;pac.dying=false;pac.dyingTimer=0;
}
static void initMaze(){
    dotsLeft=0;
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){
        maze[r][c]=MT[r][c];dots[r][c]=false;pdots[r][c]=false;
        if(MT[r][c]==0){dots[r][c]=true;dotsLeft++;}
        if(MT[r][c]==3){pdots[r][c]=true;dotsLeft++;}
    }
}
static void initGame(){
    pac.lives=3;pac.score=0;level=1;gameTime=0;
    initMaze();ghosts.clear();for(int i=0;i<4;i++)ghosts.push_back(makeGhost(i));initPac();
}
static void resetRound(){
    initPac();ghosts.clear();for(int i=0;i<4;i++)ghosts.push_back(makeGhost(i));
}

// ── Draw helpers ─────────────────────────────────────────────
static void fCirc(float cx,float cy,float r,int s=24){glBegin(GL_TRIANGLE_FAN);glVertex2f(cx,cy);for(int i=0;i<=s;i++){float a=2*PI*i/s;glVertex2f(cx+r*cosf(a),cy+r*sinf(a));}glEnd();}
static void fArc(float cx,float cy,float r,float a1,float a2,int s=24){glBegin(GL_TRIANGLE_FAN);glVertex2f(cx,cy);for(int i=0;i<=s;i++){float a=a1+(a2-a1)*i/s;glVertex2f(cx+r*cosf(a),cy+r*sinf(a));}glEnd();}
static void fRect(float x,float y,float w,float h){glBegin(GL_QUADS);glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);glEnd();}
static void dStr(float x,float y,const char* s,void* f=GLUT_BITMAP_HELVETICA_18){glRasterPos2f(x,y);for(;*s;s++)glutBitmapCharacter(f,*s);}

// ── Draw maze ────────────────────────────────────────────────
static void drawMaze(){
    float t=(float)glutGet(GLUT_ELAPSED_TIME)*0.001f;
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){
        float px=c*CELL,py=r*CELL;
        if(maze[r][c]==1){
            glColor3f(0.05f,0.05f,0.80f);fRect(px,py,CELL,CELL);
            glColor3f(0.15f,0.15f,1);glLineWidth(1.2f);
            glBegin(GL_LINE_LOOP);glVertex2f(px+1,py+1);glVertex2f(px+CELL-1,py+1);glVertex2f(px+CELL-1,py+CELL-1);glVertex2f(px+1,py+CELL-1);glEnd();
        } else if(maze[r][c]==4){glColor3f(0.8f,0.4f,0.8f);fRect(px,py+CELL/2-1.5f,CELL,3);}
    }
    for(int r=0;r<ROWS;r++)for(int c=0;c<COLS;c++){
        float px=c*CELL+CELL*0.5f,py=r*CELL+CELL*0.5f;
        if(dots[r][c]){glColor3f(0.9f,0.9f,0.6f);fCirc(px,py,2.0f,8);}
        if(pdots[r][c]){float s=0.6f+0.4f*sinf(t*5);glColor3f(1,1,s);fCirc(px,py,5.5f,16);}
    }
}

// ── Draw Pac-Man ─────────────────────────────────────────────
static void drawPacman(){
    if(pac.dying){
        float t=pac.dyingTimer/1.4f; if(t>=1) return;
        float base=0;
        if(pac.curDir==D_L)base=PI; else if(pac.curDir==D_U)base=PI/2; else if(pac.curDir==D_D)base=-PI/2;
        glColor3f(1,1,0);fArc(pac.x,pac.y,9,base+PI*t,base+2*PI-PI*t);return;
    }
    float base=0;
    if(pac.curDir==D_L)base=PI; else if(pac.curDir==D_U)base=PI/2; else if(pac.curDir==D_D)base=-PI/2;
    float open=pac.mouthAngle*PI/180.0f*0.5f;
    glColor3f(1,1,0);fArc(pac.x,pac.y,9,base+open,base+2*PI-open,32);
    glColor3f(0,0,0);fCirc(pac.x+cosf(base+PI/4)*5,pac.y+sinf(base+PI/4)*5,1.5f,8);
}

// ── Draw Ghost ───────────────────────────────────────────────
static void drawGhost(const Ghost& g){
    float px=((1-g.progress)*g.col+g.progress*g.tc+0.5f)*CELL;
    float py=((1-g.progress)*g.row+g.progress*g.tr+0.5f)*CELL;
    float rr=9.0f;
    if(g.eaten){glColor3f(1,1,1);fCirc(px-3,py+1,3);fCirc(px+3,py+1,3);glColor3f(0,0,1);fCirc(px-3,py+1,1.5f,8);fCirc(px+3,py+1,1.5f,8);return;}
    float cr=g.r,cg=g.g,cb=g.b;
    if(g.frightened){if(g.frighTimer<3&&(int)(g.frighTimer*4)%2)cr=cg=cb=0.9f;else{cr=0;cg=0;cb=0.8f;}}
    glColor3f(cr,cg,cb);fArc(px,py,rr,0,PI,20);fRect(px-rr,py,rr*2,rr);
    glBegin(GL_TRIANGLES);float sw=rr*2/3;
    for(int i=0;i<3;i++){float bx=px-rr+i*sw;glVertex2f(bx,py+rr);glVertex2f(bx+sw,py+rr);glVertex2f(bx+sw*0.5f,py+rr-5);}glEnd();
    if(!g.frightened){
        float ex=0,ey=0;
        if(g.lastDir==D_R)ex=1.5f;else if(g.lastDir==D_L)ex=-1.5f;else if(g.lastDir==D_U)ey=-1.5f;else if(g.lastDir==D_D)ey=1.5f;
        glColor3f(1,1,1);fCirc(px-3,py,3);fCirc(px+3,py,3);glColor3f(0,0,0.9f);fCirc(px-3+ex,py+ey,1.5f,8);fCirc(px+3+ex,py+ey,1.5f,8);
    } else {
        glColor3f(1,0.5f,0);glLineWidth(2);glBegin(GL_LINES);
        glVertex2f(px-5,py-2);glVertex2f(px-2,py+1);glVertex2f(px-5,py+1);glVertex2f(px-2,py-2);
        glVertex2f(px+2,py-2);glVertex2f(px+5,py+1);glVertex2f(px+2,py+1);glVertex2f(px+5,py-2);
        glEnd();glLineWidth(1);
    }
    if(!g.fire.empty()){glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);for(auto& p:g.fire){glColor4f(p.r,p.g,p.b,p.life);fCirc(p.x,p.y,3*p.life,6);}glDisable(GL_BLEND);}
}

// ── HUD (score + lives only) ──────────────────────────────────
static void drawHUD(){
    float hy=(float)(ROWS*CELL);
    glColor3f(0,0,0);fRect(0,hy,WIN_W,60);
    char buf[64];

    // Score
    glColor3f(1,1,1);sprintf(buf,"SCORE:%d",pac.score);dStr(10,hy+26,buf);

    // Best
    glColor3f(1,0.8f,0);sprintf(buf,"BEST:%d",hiScore);dStr(10,hy+48,buf,GLUT_BITMAP_HELVETICA_12);

    // Level
    glColor3f(1,1,1);sprintf(buf,"LVL:%d",level);dStr(220,hy+26,buf);

    // Lives
    glColor3f(0.6f,0.6f,0.6f);dStr(320,hy+14,"LIVES",GLUT_BITMAP_HELVETICA_12);
    for(int i=0;i<pac.lives;i++){glColor3f(1,1,0);fArc(340+i*22,hy+38,8,0.3f,2*PI-0.3f);}

    // Pause button — plain box
    bool paused=(gs==PAUSED);
    glColor3f(paused?1:0.7f,paused?1:0.7f,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(440,hy+12);glVertex2f(545,hy+12);
        glVertex2f(545,hy+46);glVertex2f(440,hy+46);
    glEnd();
    dStr(450,hy+34,paused?"[>] RESUME":"[||] PAUSE",GLUT_BITMAP_HELVETICA_12);
}

// ── Overlays ─────────────────────────────────────────────────
static void dark(float a){glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glColor4f(0,0,0,a);fRect(0,0,WIN_W,WIN_H);glDisable(GL_BLEND);}

static void drawMenu(){
    glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1,1,0);dStr(162,(float)WIN_H-60,"P A C - M A N",GLUT_BITMAP_TIMES_ROMAN_24);

    static const char* items[]={"START GAME","HELP","EXIT"};
    for(int i=0;i<3;i++){
        float y=(float)WIN_H-160-i*58;
        // plain rectangle border around each option
        glColor3f(i==menuSel?1:0.35f,i==menuSel?1:0.35f,i==menuSel?0:0.35f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(100,y-28);glVertex2f(460,y-28);
            glVertex2f(460,y+10);glVertex2f(100,y+10);
        glEnd();
        if(i==menuSel){glColor3f(1,1,0);dStr(112,y,">");}
        glColor3f(i==menuSel?1:0.5f,i==menuSel?1:0.5f,i==menuSel?0:0.5f);
        dStr(136,y,items[i]);
    }

    glColor3f(0.4f,0.4f,0.4f);dStr(118,90,"UP/DOWN  navigate     ENTER  select",GLUT_BITMAP_HELVETICA_12);
    char buf[32];sprintf(buf,"High Score: %d",hiScore);
    glColor3f(1,0.7f,0);dStr(190,58,buf,GLUT_BITMAP_HELVETICA_12);
}

static void drawHelp(){
    glClearColor(0,0,0.07f,1);glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1,1,0);dStr(190,(float)WIN_H-44,"HOW TO PLAY",GLUT_BITMAP_TIMES_ROMAN_24);
    static const char* L[]={
        "ARROW KEYS / WASD  -  Move Pac-Man",
        "P                  -  Pause / Resume",
        "ESC                -  Back to menu",
        "",
        "Eat all dots to complete the level.",
        "Avoid the 4 ghosts  -  you have 3 lives.",
        "",
        "POWER PELLET (big dot):  ghosts turn blue.",
        "  Eat a blue ghost = 200 bonus points!",
        "",
        "Each new level ghosts move a bit faster.",
        "",
        "ENTER or ESC  -  back to menu"
    };
    glColor3f(0.85f,0.85f,0.85f);
    for(int i=0;i<(int)(sizeof(L)/sizeof(L[0]));i++)
        dStr(55,(float)(WIN_H-95-i*26),L[i],GLUT_BITMAP_HELVETICA_12);
}

static void drawPause(){
    dark(0.55f);
    glColor3f(1,1,0);dStr(206,(float)WIN_H/2+28,"PAUSED",GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.8f,0.8f,0.8f);dStr(148,(float)WIN_H/2-10,"P to Resume     ESC for Menu",GLUT_BITMAP_HELVETICA_18);
}

static void drawGameOver(){
    dark(0.72f);
    glColor3f(1,0.1f,0.1f);dStr(168,(float)WIN_H/2+55,"GAME  OVER",GLUT_BITMAP_TIMES_ROMAN_24);
    char buf[40];sprintf(buf,"Score:  %d",pac.score);
    glColor3f(1,1,1);dStr(210,(float)WIN_H/2+8,buf,GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.55f,0.55f,0.55f);dStr(118,(float)WIN_H/2-32,"ENTER  play again     ESC  menu",GLUT_BITMAP_HELVETICA_12);
}

static void drawWin(){
    dark(0.72f);
    glColor3f(0,1,0.3f);dStr(180,(float)WIN_H/2+78,"YOU  WIN!",GLUT_BITMAP_TIMES_ROMAN_24);
    char buf[40];sprintf(buf,"Level %d Complete!",level);
    glColor3f(1,1,0);dStr(180,(float)WIN_H/2+38,buf,GLUT_BITMAP_HELVETICA_18);
    sprintf(buf,"Score:  %d",pac.score);
    glColor3f(1,1,1);dStr(210,(float)WIN_H/2+4,buf,GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.55f,0.55f,0.55f);dStr(118,(float)WIN_H/2-32,"ENTER  next level     ESC  menu",GLUT_BITMAP_HELVETICA_12);
}

// ── Display ──────────────────────────────────────────────────
static void display(){
    glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();
    glPushMatrix();glTranslatef(0,WIN_H,0);glScalef(1,-1,1);

    if(gs==MENU){drawMenu();glPopMatrix();glutSwapBuffers();return;}
    if(gs==HELP){drawHelp();glPopMatrix();glutSwapBuffers();return;}

    drawMaze();drawPacman();
    for(auto& g:ghosts)drawGhost(g);
    drawHUD();
    if(gs==PAUSED)   drawPause();
    if(gs==GAME_OVER)drawGameOver();
    if(gs==WIN)      drawWin();

    glPopMatrix();glutSwapBuffers();
}

// ── Pac-Man step ─────────────────────────────────────────────
static void stepPacman(float dt){
    if(pac.dying){
        pac.dyingTimer+=dt;
        if(pac.dyingTimer>1.4f){
            pac.lives--;
            if(pac.score>hiScore)hiScore=pac.score;
            if(pac.lives<=0){gs=GAME_OVER;}
            else resetRound();
        }
        return;
    }
    float tileTime=1.0f/pac.speed;
    pac.progress+=dt/tileTime;
    pac.mouthAngle+=pac.mouthDir*200.0f*dt;
    if(pac.mouthAngle>=30){pac.mouthAngle=30;pac.mouthDir=-1;}
    if(pac.mouthAngle<=2) {pac.mouthAngle=2; pac.mouthDir=1;}

    if(pac.progress>=1.0f){
        pac.progress=0;pac.col=pac.tc;pac.row=pac.tr;
        if(pac.col<0)pac.col=COLS-1;if(pac.col>=COLS)pac.col=0;
        pac.tc=pac.col;pac.tr=pac.row;
        if(dots[pac.row][pac.col]){dots[pac.row][pac.col]=false;pac.score+=10;dotsLeft--;}
        if(pdots[pac.row][pac.col]){pdots[pac.row][pac.col]=false;pac.score+=50;dotsLeft--;for(auto& g:ghosts)if(!g.inHouse&&!g.eaten){g.frightened=true;g.frighTimer=8;}}
        int nd=(int)pac.nextDir;int nc=pac.col+DC[nd],nr=pac.row+DR[nd];
        if(nc<0)nc=COLS-1;if(nc>=COLS)nc=0;
        if(pOK(nc,nr)){pac.curDir=pac.nextDir;}
        else{nd=(int)pac.curDir;nc=pac.col+DC[nd];nr=pac.row+DR[nd];if(nc<0)nc=COLS-1;if(nc>=COLS)nc=0;if(!pOK(nc,nr)){pac.x=(pac.col+0.5f)*CELL;pac.y=(pac.row+0.5f)*CELL;return;}}
        pac.tc=nc;pac.tr=nr;
    }
    float fc=pac.col*(1-pac.progress)+pac.tc*pac.progress;
    float fr=pac.row*(1-pac.progress)+pac.tr*pac.progress;
    pac.x=(fc+0.5f)*CELL;pac.y=(fr+0.5f)*CELL;
}

// ── Collision ────────────────────────────────────────────────
static void checkCol(){
    for(auto& g:ghosts){
        if(g.inHouse||g.eaten)continue;
        float dx=pac.x-g.x,dy=pac.y-g.y;
        if(dx*dx+dy*dy<(CELL*0.7f)*(CELL*0.7f)){
            if(g.frightened){g.eaten=true;g.frightened=false;pac.score+=200;}
            else if(!pac.dying){pac.dying=true;pac.dyingTimer=0;}
        }
    }
}

// ── Update ───────────────────────────────────────────────────
static void update(int){
    glutTimerFunc(16,update,0);
    if(gs!=PLAYING){glutPostRedisplay();return;}
    const float dt=0.016f;
    gameTime+=dt;
    stepPacman(dt);
    if(!pac.dying)checkCol();

    if(dotsLeft<=0){gs=WIN;if(pac.score>hiScore)hiScore=pac.score;}

    for(auto& g:ghosts){
        if(g.inHouse){
            g.houseWait-=dt;
            g.progress+=dt*g.speed;
            if(g.progress>=1.0f){
                g.progress=0;g.col=g.tc;g.row=g.tr;
                int nx=g.col+(g.lastDir==D_R?1:-1);
                if(nx<11||nx>16||maze[g.row][nx]==1)g.lastDir=(g.lastDir==D_R?D_L:D_R);
                nx=g.col+(g.lastDir==D_R?1:-1);if(nx<11||nx>16||maze[g.row][nx]==1)nx=g.col;
                g.tc=nx;g.tr=g.row;
            }
            if(g.houseWait<=0){g.col=13;g.row=11;g.tc=12;g.tr=11;g.progress=0;g.lastDir=D_L;g.inHouse=false;}
            g.x=(g.col+0.5f)*CELL;g.y=(g.row+0.5f)*CELL;continue;
        }
        updateFire(g,dt);
        if(g.frightened){g.frighTimer-=dt;if(g.frighTimer<=0)g.frightened=false;}
        float spd=g.speed*(g.frightened?0.5f:1.0f)*(g.eaten?2.5f:1.0f);
        float sv=g.speed;g.speed=spd;stepGhost(g,dt);g.speed=sv;
        if(g.eaten){
            float hx=13.5f*CELL,hy=14.5f*CELL;
            float gx=((1-g.progress)*g.col+g.progress*g.tc+0.5f)*CELL;
            float gy=((1-g.progress)*g.row+g.progress*g.tr+0.5f)*CELL;
            if(fabsf(gx-hx)<CELL&&fabsf(gy-hy)<CELL){g.eaten=false;g.frightened=false;g.col=13;g.row=14;g.tc=13;g.tr=14;g.progress=0;g.inHouse=true;g.houseWait=2;g.lastDir=D_L;}
        }
    }
    glutPostRedisplay();
}

// ── Input ────────────────────────────────────────────────────
static void specKey(int k,int,int){
    if(gs==MENU){if(k==GLUT_KEY_UP)menuSel=(menuSel+2)%3;if(k==GLUT_KEY_DOWN)menuSel=(menuSel+1)%3;glutPostRedisplay();return;}
    if(gs==PLAYING){switch(k){case GLUT_KEY_RIGHT:pac.nextDir=D_R;break;case GLUT_KEY_LEFT:pac.nextDir=D_L;break;case GLUT_KEY_UP:pac.nextDir=D_U;break;case GLUT_KEY_DOWN:pac.nextDir=D_D;break;}}
}
static void normKey(unsigned char k,int,int){
    switch(k){
    case 27: if(gs==MENU)exit(0); gs=MENU; break;
    case 'p':case 'P': if(gs==PLAYING)gs=PAUSED;else if(gs==PAUSED)gs=PLAYING; break;
    case 'd':case 'D': pac.nextDir=D_R;break;case 'a':case 'A':pac.nextDir=D_L;break;
    case 'w':case 'W': pac.nextDir=D_U;break;case 's':case 'S':pac.nextDir=D_D;break;
    case '\r':case '\n':case ' ':
        if(gs==MENU){
            if(menuSel==0){initGame();gs=PLAYING;}
            else if(menuSel==1)gs=HELP;
            else exit(0);
        } else if(gs==HELP){gs=MENU;}
        else if(gs==GAME_OVER){initGame();gs=PLAYING;}
        else if(gs==WIN){
            level++;initMaze();
            ghosts.clear();for(int i=0;i<4;i++)ghosts.push_back(makeGhost(i));
            initPac();gs=PLAYING;
        }
        break;
    }
    glutPostRedisplay();
}
static void reshape(int w,int h){glViewport(0,0,w,h);}

int main(int argc,char** argv){
    srand((unsigned)time(nullptr));
    glutInit(&argc,argv);glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(WIN_W,WIN_H);glutCreateWindow("PAC-MAN");
    glEnable(GL_LINE_SMOOTH);glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glutDisplayFunc(display);glutReshapeFunc(reshape);
    glutSpecialFunc(specKey);glutKeyboardFunc(normKey);
    glutTimerFunc(16,update,0);
    initMaze();
    glutMainLoop();return 0;
}
