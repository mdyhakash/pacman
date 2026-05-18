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
#include <vector>

// ── Constants ────────────────────────────────────────────────
static const int   CELL  = 20;
static const int   COLS  = 28;
static const int   ROWS  = 31;
static const int   WIN_W = COLS * CELL;
static const int   WIN_H = ROWS * CELL + 60;
static const float PI    = 3.14159265f;

// ── Direction enums & deltas ─────────────────────────────────
enum Dir { D_NONE=-1, D_R=0, D_U=1, D_L=2, D_D=3 };
static int DC[4] = {1,0,-1,0};
static int DR[4] = {0,-1,0,1};

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

// ── Perk struct ──────────────────────────────────────────────
struct Perk { int c,r,type; bool on; };

// ── Game state ───────────────────────────────────────────────
static int   maze[ROWS][COLS];
static int   dotsLeft = 0;
static int   score    = 0;
static bool  spdBoost = false;  static float spdT = 0;
static bool  frozen   = false;  static float frzT = 0;
static std::vector<Perk> perks;

static float pacX, pacY;
static Dir   pacDir = D_R, pacWant = D_R;
static float pacSpd = 2.5f;
static float mouthA = 0, mouthD = 1;

// ── Helpers ──────────────────────────────────────────────────
static bool isWall(int c, int r){
    if(c<0||c>=COLS||r<0||r>=ROWS) return true;
    return maze[r][c] == 1;
}
static bool pacOK(float nx, float ny){
    float rad = 7.5f;
    int cs[2] = {(int)((nx-rad)/CELL),(int)((nx+rad)/CELL)};
    int rs[2] = {(int)((ny-rad)/CELL),(int)((ny+rad)/CELL)};
    for(int i=0;i<2;i++) for(int j=0;j<2;j++)
        if(isWall(cs[i],rs[j])) return false;
    return true;
}
static float wrapX(float x){
    float W=COLS*CELL;
    if(x<0) return x+W;
    if(x>=W) return x-W;
    return x;
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
static void dStr(float x,float y,const char* s,void* f=GLUT_BITMAP_HELVETICA_18){
    glRasterPos2f(x,y); for(;*s;s++) glutBitmapCharacter(f,*s);
}

// ── Init ─────────────────────────────────────────────────────
static void initMaze(){
    dotsLeft=0;
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++){
        maze[r][c]=MT[r][c];
        if(maze[r][c]==0||maze[r][c]==3) dotsLeft++;
    }
    perks.clear();
    perks.push_back({6,11,0,true});
    perks.push_back({21,11,1,true});
    pacX=13.5f*CELL; pacY=23.5f*CELL;
    pacDir=D_R; pacWant=D_R;
    score=0; spdBoost=false; frozen=false;
}

// ── Draw maze ────────────────────────────────────────────────
static void drawMaze(){
    float t=(float)glutGet(GLUT_ELAPSED_TIME)*0.001f;
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++){
        float px=c*CELL, py=r*CELL;
        int v=maze[r][c];
        if(v==1){
            glColor3f(0.05f,0.05f,0.80f); fRect(px,py,CELL,CELL);
            glColor3f(0.15f,0.15f,1); glLineWidth(1.2f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(px+1,py+1); glVertex2f(px+CELL-1,py+1);
            glVertex2f(px+CELL-1,py+CELL-1); glVertex2f(px+1,py+CELL-1);
            glEnd();
        } else if(v==0){
            glColor3f(0.9f,0.9f,0.6f); fCirc(px+CELL/2,py+CELL/2,2.0f);
        } else if(v==3){
            float s=0.6f+0.4f*sinf(t*5);
            glColor3f(1,1,s); fCirc(px+CELL/2,py+CELL/2,5.5f);
        } else if(v==4){
            glColor3f(0.8f,0.4f,0.8f); fRect(px,py+CELL/2-1.5f,CELL,3);
        }
    }
    // Perks
    for(auto& p:perks){
        if(!p.on) continue;
        float px=p.c*CELL+CELL/2, py=p.r*CELL+CELL/2;
        if(p.type==0){
            glColor3f(1,0.9f,0);
            glBegin(GL_TRIANGLE_FAN); glVertex2f(px,py);
            for(int i=0;i<=10;i++){float a=PI/2+i*2*PI/10;float rr=(i%2==0)?7.0f:3.0f;glVertex2f(px+rr*cosf(a),py+rr*sinf(a));}
            glEnd();
        } else {
            glColor3f(0.4f,0.8f,1); glLineWidth(2);
            glBegin(GL_LINES);
            for(int i=0;i<4;i++){float a=i*PI/4;glVertex2f(px-6*cosf(a),py-6*sinf(a));glVertex2f(px+6*cosf(a),py+6*sinf(a));}
            glEnd(); glLineWidth(1);
        }
    }
}

// ── Draw Pac-Man ─────────────────────────────────────────────
static void drawPac(){
    float base=0;
    if(pacDir==D_U) base=PI/2;
    else if(pacDir==D_L) base=PI;
    else if(pacDir==D_D) base=-PI/2;
    float open=(mouthA*PI/180)*0.5f;
    glColor3f(1,1,0); fArc(pacX,pacY,9,base+open,base+2*PI-open);
    glColor3f(0,0,0); fCirc(pacX+cosf(base+PI/4)*5,pacY+sinf(base+PI/4)*5,1.5f,8);
    if(spdBoost){
        glColor3f(1,0.9f,0); glLineWidth(2);
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<16;i++){float a=2*PI*i/16;glVertex2f(pacX+13*cosf(a),pacY+13*sinf(a));}
        glEnd(); glLineWidth(1);
    }
}

// ── Draw HUD ─────────────────────────────────────────────────
static void drawHUD(){
    float hy=ROWS*CELL;
    glColor3f(0,0,0); fRect(0,hy,WIN_W,60);
    char buf[64];
    glColor3f(1,1,1);
    sprintf(buf,"SCORE %d",score); dStr(10,hy+22,buf);
    sprintf(buf,"DOTS LEFT %d",dotsLeft); dStr(195,hy+22,buf);
    if(spdBoost){glColor3f(1,1,0); dStr(10,hy+48,"SPD BOOST",GLUT_BITMAP_HELVETICA_12);}
    if(frozen)  {glColor3f(0.4f,0.8f,1); dStr(120,hy+48,"FROZEN",GLUT_BITMAP_HELVETICA_12);}
    glColor3f(0.5f,0.5f,0.5f);
    dStr(320,hy+48,"Arrow keys to move | R = reset",GLUT_BITMAP_HELVETICA_12);
}

// ── Display ──────────────────────────────────────────────────
static void display(){
    glClearColor(0,0,0,1); glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW);  glLoadIdentity();
    glPushMatrix();
    glTranslatef(0,WIN_H,0); glScalef(1,-1,1);
    drawMaze();
    drawPac();
    drawHUD();
    glPopMatrix();
    glutSwapBuffers();
}

// ── Update ───────────────────────────────────────────────────
static void update(int){
    glutTimerFunc(16,update,0);
    const float dt=0.016f;
    float ps=pacSpd*(spdBoost?1.6f:1.0f);

    // Try queued direction
    if(pacWant!=pacDir){
        int pd=(int)pacWant;
        float nx=pacX+DC[pd]*ps, ny=pacY+DR[pd]*ps;
        if(pacOK(nx,ny)) pacDir=pacWant;
    }
    // Move
    {
        int pd=(int)pacDir;
        float nx=pacX+DC[pd]*ps, ny=pacY+DR[pd]*ps;
        if(pacOK(nx,ny)){pacX=wrapX(nx);pacY=ny;}
    }
    mouthA+=mouthD*5;
    if(mouthA>45) mouthD=-1;
    if(mouthA<0){mouthA=0;mouthD=1;}

    // Eat dots
    int cc=(int)(pacX/CELL), rr=(int)(pacY/CELL);
    for(int dr=-1;dr<=1;dr++) for(int dc=-1;dc<=1;dc++){
        int nc=cc+dc, nr=rr+dr;
        if(nc<0||nc>=COLS||nr<0||nr>=ROWS) continue;
        float cx=(nc+0.5f)*CELL, cy=(nr+0.5f)*CELL;
        float d2=(pacX-cx)*(pacX-cx)+(pacY-cy)*(pacY-cy);
        if(d2>(CELL*0.55f)*(CELL*0.55f)) continue;
        if(maze[nr][nc]==0){maze[nr][nc]=2;score+=10;dotsLeft--;}
        else if(maze[nr][nc]==3){maze[nr][nc]=2;score+=50;dotsLeft--;}
    }

    // Perks
    for(auto& pk:perks){
        if(!pk.on) continue;
        float cx=(pk.c+0.5f)*CELL, cy=(pk.r+0.5f)*CELL;
        float d2=(pacX-cx)*(pacX-cx)+(pacY-cy)*(pacY-cy);
        if(d2<(CELL*0.7f)*(CELL*0.7f)){
            pk.on=false; score+=100;
            if(pk.type==0){spdBoost=true;spdT=5;}
            else          {frozen=true;frzT=4;}
        }
    }
    if(spdBoost){spdT-=dt;if(spdT<=0)spdBoost=false;}
    if(frozen)  {frzT-=dt;if(frzT<=0)frozen=false;}

    if(dotsLeft<=0) initMaze(); // auto-reset when all dots eaten

    glutPostRedisplay();
}

// ── Input ────────────────────────────────────────────────────
static void specKey(int k,int,int){
    switch(k){
        case GLUT_KEY_RIGHT: pacWant=D_R; break;
        case GLUT_KEY_LEFT:  pacWant=D_L; break;
        case GLUT_KEY_UP:    pacWant=D_U; break;
        case GLUT_KEY_DOWN:  pacWant=D_D; break;
    }
}
static void normKey(unsigned char k,int,int){
    if(k==27) exit(0);
    if(k=='r'||k=='R') initMaze();
}
static void reshape(int w,int h){ glViewport(0,0,w,h); }

// ── Main ─────────────────────────────────────────────────────
int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(WIN_W,WIN_H);
    glutCreateWindow("PAC-MAN | Member 1 - Maze & Movement");
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specKey);
    glutKeyboardFunc(normKey);
    glutTimerFunc(16,update,0);
    initMaze();
    glutMainLoop();
    return 0;
}
