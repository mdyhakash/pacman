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

// ── Constants ────────────────────────────────────────────────
static const int   CELL  = 20;
static const int   COLS  = 28;
static const int   ROWS  = 31;
static const int   WIN_W = COLS * CELL;
static const int   WIN_H = ROWS * CELL + 50;
static const float PI    = 3.14159265f;

// ── Direction enums & deltas ─────────────────────────────────
enum Dir { D_NONE=-1, D_R=0, D_U=1, D_L=2, D_D=3 };
static int DC[4] = {1,0,-1,0};
static int DR[4] = {0,-1,0,1};

static Dir opposite(Dir d){
    switch(d){case D_R:return D_L;case D_L:return D_R;
              case D_U:return D_D;case D_D:return D_U;default:return D_NONE;}
}

// ── Maze template ────────────────────────────────────────────
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

// ── Structs ──────────────────────────────────────────────────
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

// ── Maze helpers ─────────────────────────────────────────────
static bool isWall(int c,int r){
    if(c<0||c>=COLS||r<0||r>=ROWS) return true;
    return maze[r][c]==1;
}
static bool isDoor(int c,int r){
    if(c<0||c>=COLS||r<0||r>=ROWS) return false;
    return maze[r][c]==4;
}
static bool gOK(int c,int r,bool allowDoor=false){
    if(c<0||c>=COLS||r<0||r>=ROWS) return false;
    if(maze[r][c]==1) return false;
    if(isDoor(c,r)&&!allowDoor) return false;
    return true;
}

// ── Ghost AI state ────────────────────────────────────────────
// Pac-Man is replaced by a dummy moving target for this demo
static float dummyX = 13.5f*CELL;
static float dummyY = 23.5f*CELL;
static float dummyAngle = 0;
static std::vector<Ghost> ghosts;

// ── Pathfinding ───────────────────────────────────────────────
static void pickNext(Ghost& g, float tx, float ty, bool allowDoor){
    int gc=g.tc, gr=g.tr;
    Dir noGo=opposite(g.lastDir);
    int bestC=gc,bestR=gr; float bestD=1e18f; bool found=false;
    for(int d=0;d<4;d++){
        if((Dir)d==noGo) continue;
        int nc=gc+DC[d],nr=gr+DR[d];
        if(!gOK(nc,nr,allowDoor)) continue;
        float dx=(nc+0.5f)*CELL-tx, dy=(nr+0.5f)*CELL-ty;
        float dd=dx*dx+dy*dy;
        if(!found||dd<bestD){bestD=dd;bestC=nc;bestR=nr;found=true;}
    }
    if(!found){
        Dir rev=opposite(g.lastDir);
        if(rev!=D_NONE){int nc=gc+DC[(int)rev],nr=gr+DR[(int)rev];if(gOK(nc,nr,allowDoor)){bestC=nc;bestR=nr;found=true;}}
    }
    if(!found){for(int d=0;d<4;d++){int nc=gc+DC[d],nr=gr+DR[d];if(gOK(nc,nr,allowDoor)){bestC=nc;bestR=nr;found=true;break;}}}
    int ddc=bestC-gc,ddr=bestR-gr;
    g.lastDir=D_NONE;
    for(int d=0;d<4;d++) if(DC[d]==ddc&&DR[d]==ddr){g.lastDir=(Dir)d;break;}
    g.col=gc;g.row=gr;g.tc=bestC;g.tr=bestR;g.progress=0;
}

static void pickNextRandom(Ghost& g, bool allowDoor){
    int gc=g.tc,gr=g.tr;
    Dir noGo=opposite(g.lastDir);
    int valid[4]; int nv=0;
    for(int d=0;d<4;d++){
        if((Dir)d==noGo) continue;
        int nc=gc+DC[d],nr=gr+DR[d];
        if(gOK(nc,nr,allowDoor)) valid[nv++]=d;
    }
    int chosen=-1;
    if(nv>0) chosen=valid[rand()%nv];
    else for(int d=0;d<4;d++){int nc=gc+DC[d],nr=gr+DR[d];if(gOK(nc,nr,allowDoor)){chosen=d;break;}}
    if(chosen<0){g.col=gc;g.row=gr;g.tc=gc;g.tr=gr;g.progress=0;return;}
    g.lastDir=(Dir)chosen;
    g.col=gc;g.row=gr;g.tc=gc+DC[chosen];g.tr=gr+DR[chosen];g.progress=0;
}

// ── Ghost target ─────────────────────────────────────────────
static void ghostTarget(const Ghost& g, float& tx, float& ty){
    // id 0: direct chase
    if(g.id==0){tx=dummyX;ty=dummyY;return;}
    // id 1: 4 tiles ahead
    if(g.id==1){tx=dummyX+DC[D_R]*4*CELL;ty=dummyY+DR[D_R]*4*CELL;return;}
    // id 2: flanking
    if(g.id==2 && !ghosts.empty()){
        float p2x=dummyX+DC[D_R]*2*CELL, p2y=dummyY+DR[D_R]*2*CELL;
        tx=2*p2x-ghosts[0].x; ty=2*p2y-ghosts[0].y; return;
    }
    // id 3: scatter when close
    if(g.id==3){
        float d2=(g.col*CELL+CELL/2-dummyX)*(g.col*CELL+CELL/2-dummyX)+
                 (g.row*CELL+CELL/2-dummyY)*(g.row*CELL+CELL/2-dummyY);
        if(d2>(8*CELL)*(8*CELL)){tx=dummyX;ty=dummyY;}
        else{tx=1*CELL;ty=29*CELL;}
        return;
    }
    tx=dummyX;ty=dummyY;
}

// ── Fire particles ────────────────────────────────────────────
static void updateFire(Ghost& g, float dt){
    if(g.id!=0||g.inHouse||g.eaten||g.frightened) return;
    g.fireT-=dt;
    if(g.fireT<=0){
        g.fireT=1.5f+(float)(rand()%3);
        float px=((1-g.progress)*g.col+g.progress*g.tc+0.5f)*CELL;
        float py=((1-g.progress)*g.row+g.progress*g.tr+0.5f)*CELL;
        for(int i=0;i<10;i++){
            Pt p;p.x=px;p.y=py;
            float a=rand()%360*PI/180; float sp=12+(rand()%20);
            p.vx=cosf(a)*sp;p.vy=sinf(a)*sp;p.life=1;
            p.r=1;p.g=0.1f+(rand()%6)*0.1f;p.b=0;
            g.fire.push_back(p);
        }
    }
    for(auto& p:g.fire){p.x+=p.vx*dt;p.y+=p.vy*dt;p.life-=dt*1.3f;}
    g.fire.erase(std::remove_if(g.fire.begin(),g.fire.end(),[](const Pt& p){return p.life<=0;}),g.fire.end());
}

// ── Step ghost ────────────────────────────────────────────────
static void stepGhost(Ghost& g, float dt){
    float tileTime=1.0f/g.speed;
    g.progress+=dt/tileTime;
    if(g.progress>=1.0f){
        g.progress=1.0f;
        g.x=(g.tc+0.5f)*CELL;
        g.y=(g.tr+0.5f)*CELL;
        bool allowDoor=g.eaten;
        if(g.frightened) pickNextRandom(g,allowDoor);
        else{float tx,ty;ghostTarget(g,tx,ty);pickNext(g,tx,ty,allowDoor);}
    }
}

// ── Ghost factory ─────────────────────────────────────────────
static Ghost makeGhost(int id){
    Ghost g;
    g.id=id;g.progress=0;g.frightened=false;g.frighTimer=0;
    g.eaten=false;g.inHouse=true;g.fireT=2.0f;g.fire.clear();g.lastDir=D_L;
    g.speed=3.0f;
    static const int SC[4]={13,13,11,15};
    static const int SR[4]={14,13,13,13};
    g.col=SC[id];g.row=SR[id];g.tc=SC[id];g.tr=SR[id];
    g.houseWait=(float)id*3.0f;
    switch(id){
        case 0:g.r=1;g.g=0.1f;g.b=0.1f;break;
        case 1:g.r=1;g.g=0.6f;g.b=0.8f;break;
        case 2:g.r=0;g.g=0.9f;g.b=0.9f;break;
        case 3:g.r=1;g.g=0.6f;g.b=0.1f;break;
    }
    g.x=(g.col+0.5f)*CELL;
    g.y=(g.row+0.5f)*CELL;
    return g;
}

// ── Draw helpers ─────────────────────────────────────────────
static void fCirc(float cx,float cy,float r,int s=24){
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
    for(int i=0;i<=s;i++){float a=2*PI*i/s;glVertex2f(cx+r*cosf(a),cy+r*sinf(a));}
    glEnd();
}
static void fArc(float cx,float cy,float r,float a1,float a2,int s=24){
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
    for(int i=0;i<=s;i++){float a=a1+(a2-a1)*i/s;glVertex2f(cx+r*cosf(a),cy+r*sinf(a));}
    glEnd();
}
static void fRect(float x,float y,float w,float h){
    glBegin(GL_QUADS);
    glVertex2f(x,y);glVertex2f(x+w,y);glVertex2f(x+w,y+h);glVertex2f(x,y+h);
    glEnd();
}
static void dStr(float x,float y,const char* s,void* f=GLUT_BITMAP_HELVETICA_12){
    glRasterPos2f(x,y); for(;*s;s++) glutBitmapCharacter(f,*s);
}

// ── Draw maze (walls only, no dots for clarity) ───────────────
static void drawMazeBg(){
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++){
        float px=c*CELL,py=r*CELL;
        if(maze[r][c]==1){
            glColor3f(0.05f,0.05f,0.70f);fRect(px,py,CELL,CELL);
            glColor3f(0.1f,0.1f,0.9f);glLineWidth(1.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(px+1,py+1);glVertex2f(px+CELL-1,py+1);
            glVertex2f(px+CELL-1,py+CELL-1);glVertex2f(px+1,py+CELL-1);
            glEnd();
        } else if(maze[r][c]==4){
            glColor3f(0.8f,0.4f,0.8f);fRect(px,py+CELL/2-1.5f,CELL,3);
        }
    }
}

// ── Draw a ghost ─────────────────────────────────────────────
static void drawGhost(const Ghost& g){
    float px=((1-g.progress)*g.col+g.progress*g.tc+0.5f)*CELL;
    float py=((1-g.progress)*g.row+g.progress*g.tr+0.5f)*CELL;
    float rr=9.0f;

    if(g.eaten){
        glColor3f(1,1,1);fCirc(px-3,py+1,3);fCirc(px+3,py+1,3);
        glColor3f(0,0,1);fCirc(px-3,py+1,1.5f,8);fCirc(px+3,py+1,1.5f,8);
        return;
    }
    float cr=g.r,cg=g.g,cb=g.b;
    if(g.frightened){
        if(g.frighTimer<3&&(int)(g.frighTimer*4)%2)cr=cg=cb=0.9f;
        else{cr=0;cg=0;cb=0.8f;}
    }
    glColor3f(cr,cg,cb);
    fArc(px,py,rr,0,PI,20);
    fRect(px-rr,py,rr*2,rr);
    glBegin(GL_TRIANGLES);
    float sw=rr*2/3;
    for(int i=0;i<3;i++){float bx=px-rr+i*sw;glVertex2f(bx,py+rr);glVertex2f(bx+sw,py+rr);glVertex2f(bx+sw*0.5f,py+rr-5);}
    glEnd();
    if(!g.frightened){
        float ex=0,ey=0;
        if(g.lastDir==D_R)ex=1.5f; else if(g.lastDir==D_L)ex=-1.5f;
        else if(g.lastDir==D_U)ey=-1.5f; else if(g.lastDir==D_D)ey=1.5f;
        glColor3f(1,1,1);fCirc(px-3,py,3);fCirc(px+3,py,3);
        glColor3f(0,0,0.9f);fCirc(px-3+ex,py+ey,1.5f,8);fCirc(px+3+ex,py+ey,1.5f,8);
    }
    if(!g.fire.empty()){
        glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        for(auto& p:g.fire){glColor4f(p.r,p.g,p.b,p.life);fCirc(p.x,p.y,3*p.life,6);}
        glDisable(GL_BLEND);
    }
}

// ── HUD ──────────────────────────────────────────────────────
static void drawHUD(){
    float hy=ROWS*CELL;
    glColor3f(0,0,0);fRect(0,hy,WIN_W,50);
    glColor3f(1,1,1);
    dStr(10,hy+18,"GHOST AI DEMO  |  Click = frighten  |  SPACE = eat all  |  R = reset");
    // Ghost name labels
    static const char* names[]={"Blinky (direct chase)","Pinky (4-ahead)","Inky (flanking)","Clyde (scatter)"};
    static const float cr[]={1,1,0,1}, cg[]={0.1f,0.6f,0.9f,0.6f}, cb[]={0.1f,0.8f,0.9f,0.1f};
    for(int i=0;i<4&&i<(int)ghosts.size();i++){
        glColor3f(cr[i],cg[i],cb[i]);
        char buf[64]; sprintf(buf,"%s",names[i]);
        dStr(10+i*140, hy+36, buf);
    }
}

// ── Display ──────────────────────────────────────────────────
static void display(){
    glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();
    glPushMatrix();
    glTranslatef(0,WIN_H,0);glScalef(1,-1,1);
    drawMazeBg();
    // Draw dummy target (yellow dot = simulated pac position)
    glColor3f(1,1,0);fCirc(dummyX,dummyY,7);
    for(auto& g:ghosts) drawGhost(g);
    drawHUD();
    glPopMatrix();
    glutSwapBuffers();
}

// ── Update ───────────────────────────────────────────────────
static void update(int){
    glutTimerFunc(16,update,0);
    const float dt=0.016f;

    // Move dummy target in a slow circle around the maze
    dummyAngle+=dt*0.4f;
    dummyX=13.5f*CELL+cosf(dummyAngle)*5*CELL;
    dummyY=14.5f*CELL+sinf(dummyAngle)*4*CELL;

    for(auto& g:ghosts){
        if(g.inHouse){
            g.houseWait-=dt;
            g.progress+=dt*g.speed;
            if(g.progress>=1.0f){
                g.progress=0;g.col=g.tc;g.row=g.tr;
                int nextC=g.col+(g.lastDir==D_R?1:-1);
                if(nextC<11||nextC>16||maze[g.row][nextC]==1)g.lastDir=(g.lastDir==D_R?D_L:D_R);
                nextC=g.col+(g.lastDir==D_R?1:-1);
                if(nextC<11||nextC>16||maze[g.row][nextC]==1)nextC=g.col;
                g.tc=nextC;g.tr=g.row;
            }
            if(g.houseWait<=0){
                g.col=13;g.row=11;g.tc=12;g.tr=11;g.progress=0;g.lastDir=D_L;g.inHouse=false;
            }
            g.x=(g.col+0.5f)*CELL;g.y=(g.row+0.5f)*CELL;
            continue;
        }
        updateFire(g,dt);
        if(g.frightened){g.frighTimer-=dt;if(g.frighTimer<=0)g.frightened=false;}
        float spd=g.speed*(g.frightened?0.5f:1.0f)*(g.eaten?2.5f:1.0f);
        float saved=g.speed; g.speed=spd;
        stepGhost(g,dt);
        g.speed=saved;
        // Eaten: return home
        if(g.eaten){
            float hx=13.5f*CELL,hy=14.5f*CELL;
            float px=((1-g.progress)*g.col+g.progress*g.tc+0.5f)*CELL;
            float py=((1-g.progress)*g.row+g.progress*g.tr+0.5f)*CELL;
            if(fabsf(px-hx)<CELL&&fabsf(py-hy)<CELL&&!g.inHouse){
                g.eaten=false;g.frightened=false;
                g.col=13;g.row=14;g.tc=13;g.tr=14;g.progress=0;
                g.inHouse=true;g.houseWait=2;g.lastDir=D_L;
            }
        }
    }
    glutPostRedisplay();
}

// ── Init ─────────────────────────────────────────────────────
static void init(){
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++) maze[r][c]=MT[r][c];
    ghosts.clear();
    for(int i=0;i<4;i++) ghosts.push_back(makeGhost(i));
}

// ── Input ────────────────────────────────────────────────────
static void mouse(int btn,int state,int,int){
    if(btn==GLUT_LEFT_BUTTON&&state==GLUT_DOWN)
        for(auto& g:ghosts)if(!g.inHouse&&!g.eaten){g.frightened=true;g.frighTimer=8;}
}
static void normKey(unsigned char k,int,int){
    if(k==27) exit(0);
    if(k=='r'||k=='R') init();
    if(k==' ') for(auto& g:ghosts)if(!g.inHouse&&!g.eaten){g.eaten=true;g.frightened=false;}
}
static void reshape(int w,int h){glViewport(0,0,w,h);}

// ── Main ─────────────────────────────────────────────────────
int main(int argc,char** argv){
    srand((unsigned)time(nullptr));
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(WIN_W,WIN_H);
    glutCreateWindow("PAC-MAN | Member 2 - Ghost AI Demo");
    glEnable(GL_LINE_SMOOTH);glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(normKey);
    glutMouseFunc(mouse);
    glutTimerFunc(16,update,0);
    init();
    glutMainLoop();
    return 0;
}