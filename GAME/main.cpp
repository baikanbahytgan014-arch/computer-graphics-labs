#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/gl.h>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <random>
#include <string>
#include <vector>

namespace {
constexpr int W=900,H=620,TOP=76,BOTTOM=H-28;
constexpr GLenum VERTEX_SHADER=0x8B31,FRAGMENT_SHADER=0x8B30;
constexpr GLenum ARRAY_BUFFER=0x8892,DYNAMIC_DRAW=0x88E8,COMPILE_STATUS=0x8B81,LINK_STATUS=0x8B82;
struct Vertex { float x,y,r,g,b; };
struct Object { float x,y,speed,size; bool pixel; };

using CreateContext=HGLRC(WINAPI*)(HDC,HGLRC,const int*);
using CreateShader=GLuint(APIENTRY*)(GLenum); using ShaderSource=void(APIENTRY*)(GLuint,GLsizei,const char*const*,const GLint*);
using CompileShader=void(APIENTRY*)(GLuint); using GetShaderiv=void(APIENTRY*)(GLuint,GLenum,GLint*);
using GetShaderLog=void(APIENTRY*)(GLuint,GLsizei,GLsizei*,char*); using CreateProgram=GLuint(APIENTRY*)();
using AttachShader=void(APIENTRY*)(GLuint,GLuint); using LinkProgram=void(APIENTRY*)(GLuint);
using GetProgramiv=void(APIENTRY*)(GLuint,GLenum,GLint*); using GetProgramLog=void(APIENTRY*)(GLuint,GLsizei,GLsizei*,char*);
using DeleteShader=void(APIENTRY*)(GLuint); using DeleteProgram=void(APIENTRY*)(GLuint);
using UseProgram=void(APIENTRY*)(GLuint); using GetUniform=GLint(APIENTRY*)(GLuint,const char*);
using Uniform2f=void(APIENTRY*)(GLint,GLfloat,GLfloat); using GenVertexArrays=void(APIENTRY*)(GLsizei,GLuint*);
using BindVertexArray=void(APIENTRY*)(GLuint); using DeleteVertexArrays=void(APIENTRY*)(GLsizei,const GLuint*);
using GenBuffers=void(APIENTRY*)(GLsizei,GLuint*); using BindBuffer=void(APIENTRY*)(GLenum,GLuint);
using BufferData=void(APIENTRY*)(GLenum,std::ptrdiff_t,const void*,GLenum);
using DeleteBuffers=void(APIENTRY*)(GLsizei,const GLuint*); using EnableAttrib=void(APIENTRY*)(GLuint);
using AttribPointer=void(APIENTRY*)(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*);
using DrawArrays=void(APIENTRY*)(GLenum,GLint,GLsizei);

CreateShader pCreateShader{}; ShaderSource pShaderSource{}; CompileShader pCompileShader{};
GetShaderiv pGetShaderiv{}; GetShaderLog pGetShaderLog{}; CreateProgram pCreateProgram{};
AttachShader pAttachShader{}; LinkProgram pLinkProgram{}; GetProgramiv pGetProgramiv{};
GetProgramLog pGetProgramLog{}; DeleteShader pDeleteShader{}; DeleteProgram pDeleteProgram{};
UseProgram pUseProgram{}; GetUniform pGetUniform{}; Uniform2f pUniform2f{};
GenVertexArrays pGenVertexArrays{}; BindVertexArray pBindVertexArray{}; DeleteVertexArrays pDeleteVertexArrays{};
GenBuffers pGenBuffers{}; BindBuffer pBindBuffer{}; BufferData pBufferData{}; DeleteBuffers pDeleteBuffers{};
EnableAttrib pEnableAttrib{}; AttribPointer pAttribPointer{}; DrawArrays pDrawArrays{};

HWND window{}; HDC dc{}; HGLRC context{}; GLuint program{},vao{},vbo{}; GLint resolution{};
const wchar_t* glError=L"Не удалось создать OpenGL-контекст.";
bool keys[256]{}; int screen=0,score=0,lives=3,spawn=0;
float px=W/2.0f,py=H-105.0f,shield=0;
std::vector<Object> objects; std::vector<Vertex> vertices;
std::mt19937 rng{std::random_device{}()};
auto previous=std::chrono::steady_clock::now();

template<class T> T loadGL(const char* name){
    PROC p=wglGetProcAddress(name);
    if(!p||p==reinterpret_cast<PROC>(1)||p==reinterpret_cast<PROC>(2)||
       p==reinterpret_cast<PROC>(3)||p==reinterpret_cast<PROC>(-1)){
        static HMODULE dll=LoadLibraryW(L"opengl32.dll"); p=reinterpret_cast<PROC>(GetProcAddress(dll,name));
    }
    T result{};
    static_assert(sizeof(result)==sizeof(p), "OpenGL function pointer size mismatch");
    std::memcpy(&result,&p,sizeof(result));
    return result;
}
bool loadFunctions(){
#define LOAD(name) p##name=loadGL<name>("gl" #name); if(!p##name)return false
    LOAD(CreateShader);LOAD(ShaderSource);LOAD(CompileShader);LOAD(GetShaderiv);
    pGetShaderLog=loadGL<GetShaderLog>("glGetShaderInfoLog");if(!pGetShaderLog)return false;
    LOAD(CreateProgram);LOAD(AttachShader);LOAD(LinkProgram);LOAD(GetProgramiv);
    pGetProgramLog=loadGL<GetProgramLog>("glGetProgramInfoLog");if(!pGetProgramLog)return false;
    LOAD(DeleteShader);LOAD(DeleteProgram);LOAD(UseProgram);
    pGetUniform=loadGL<GetUniform>("glGetUniformLocation");if(!pGetUniform)return false;
    LOAD(Uniform2f);
    LOAD(GenVertexArrays);LOAD(BindVertexArray);LOAD(DeleteVertexArrays);LOAD(GenBuffers);LOAD(BindBuffer);
    LOAD(BufferData);LOAD(DeleteBuffers);LOAD(DrawArrays);
    pEnableAttrib=loadGL<EnableAttrib>("glEnableVertexAttribArray");if(!pEnableAttrib)return false;
    pAttribPointer=loadGL<AttribPointer>("glVertexAttribPointer");if(!pAttribPointer)return false;
#undef LOAD
    return true;
}
GLuint makeShader(GLenum type,const char* source){
    GLuint s=pCreateShader(type);pShaderSource(s,1,&source,nullptr);pCompileShader(s);GLint ok=0;pGetShaderiv(s,COMPILE_STATUS,&ok);
    if(!ok){char log[1024]{};pGetShaderLog(s,sizeof(log),nullptr,log);MessageBoxA(window,log,"Shader compile error",MB_ICONERROR);return 0;}
    return s;
}

void quad(float x1,float y1,float x2,float y2,float r,float g,float b){
    Vertex a{x1,y1,r,g,b},c{x2,y1,r,g,b},d{x2,y2,r,g,b},e{x1,y2,r,g,b};
    vertices.insert(vertices.end(),{a,c,d,a,d,e});
}

const char* glyph(char c){
    static const char* f[]={
    "01110/10001/10001/11111/10001/10001/10001","11110/10001/10001/11110/10001/10001/11110",
    "01111/10000/10000/10000/10000/10000/01111","11110/10001/10001/10001/10001/10001/11110",
    "11111/10000/10000/11110/10000/10000/11111","11111/10000/10000/11110/10000/10000/10000",
    "01111/10000/10000/10111/10001/10001/01111","10001/10001/10001/11111/10001/10001/10001",
    "11111/00100/00100/00100/00100/00100/11111","00111/00010/00010/00010/10010/10010/01100",
    "10001/10010/10100/11000/10100/10010/10001","10000/10000/10000/10000/10000/10000/11111",
    "10001/11011/10101/10101/10001/10001/10001","10001/11001/10101/10011/10001/10001/10001",
    "01110/10001/10001/10001/10001/10001/01110","11110/10001/10001/11110/10000/10000/10000",
    "01110/10001/10001/10001/10101/10010/01101","11110/10001/10001/11110/10100/10010/10001",
    "01111/10000/10000/01110/00001/00001/11110","11111/00100/00100/00100/00100/00100/00100",
    "10001/10001/10001/10001/10001/10001/01110","10001/10001/10001/10001/10001/01010/00100",
    "10001/10001/10001/10101/10101/10101/01010","10001/10001/01010/00100/01010/10001/10001",
    "10001/10001/01010/00100/00100/00100/00100","11111/00001/00010/00100/01000/10000/11111",
    "01110/10001/10011/10101/11001/10001/01110","00100/01100/00100/00100/00100/00100/01110",
    "01110/10001/00001/00010/00100/01000/11111","11110/00001/00001/01110/00001/00001/11110",
    "00010/00110/01010/10010/11111/00010/00010","11111/10000/10000/11110/00001/00001/11110",
    "01110/10000/10000/11110/10001/10001/01110","11111/00001/00010/00100/01000/01000/01000",
    "01110/10001/10001/01110/10001/10001/01110","01110/10001/10001/01111/00001/00001/01110"};
    if(c>='A'&&c<='Z')return f[c-'A'];
    if(c>='0'&&c<='9')return f[26+c-'0'];
    return nullptr;
}
void label(float x,float y,const std::string& s,int scale,float r,float g,float b){
    for(char c:s){if(c==' '){x+=6*scale;continue;}const char* p=glyph(c);if(!p){x+=6*scale;continue;}
        for(int row=0;row<7;row++)for(int col=0;col<5;col++)if(p[row*6+col]=='1')quad(x+col*scale,y+row*scale,x+(col+1)*scale,y+(row+1)*scale,r,g,b);
        x+=6*scale;
    }
}
void reset(){px=W/2.0f;py=H-105;score=0;lives=3;spawn=0;shield=0;objects.clear();screen=1;}
void buildFrame(){
    vertices.clear();quad(0,0,W,H,.035f,.055f,.095f);quad(0,0,W,TOP,.065f,.095f,.15f);
    for(int x=0;x<W;x+=40)quad(x,TOP,x+1,H,.075f,.11f,.17f);
    for(int y=TOP;y<H;y+=40)quad(0,y,W,y+1,.075f,.11f,.17f);
    label(22,22,"PIXEL PATROL",3,.4f,.96f,.9f);label(450,25,"SCORE",2,.95f,.97f,1);
    label(570,25,std::to_string(score),2,.95f,.97f,1);label(690,25,"LIVES",2,1,.43f,.7f);
    for(int i=0;i<lives;i++)quad(810+i*22,22,826+i*22,38,1,.43f,.7f);
    for(auto o:objects){if(o.pixel)quad(o.x-o.size/2,o.y-o.size/2,o.x+o.size/2,o.y+o.size/2,.4f,.96f,.9f);
        else {float s=o.size;Vertex a{o.x,o.y-s,1,.3f,.48f},b{o.x+s,o.y,1,.3f,.48f},c{o.x,o.y+s,1,.3f,.48f},d{o.x-s,o.y,1,.3f,.48f};vertices.insert(vertices.end(),{a,b,c,a,c,d});}}
    float blue=shield>0&&int(shield*12)%2==0?1:.4f;
    Vertex a{px,py-23,blue,1,.9f},b{px+19,py+17,blue,1,.9f},c{px,py+8,blue,1,.9f},d{px-19,py+17,blue,1,.9f};vertices.insert(vertices.end(),{a,b,c,a,c,d});
    label(205,585,"COLLECT PIXELS +10  -  AVOID RED GLITCHES",2,.58f,.66f,.78f);
    if(screen!=1){quad(0,0,W,H,.025f,.04f,.075f);
        if(screen==0){label(280,230,"PIXEL PATROL",5,.4f,.96f,.9f);label(250,300,"COLLECT PIXELS",3,.9f,.94f,1);label(250,340,"AVOID GLITCHES",3,1,.35f,.5f);label(280,410,"WASD OR ARROWS",2,1,.82f,.42f);label(340,455,"SPACE TO START",2,.4f,.96f,.9f);}
        else{label(300,260,"GAME OVER",4,1,.43f,.7f);label(330,325,"SCORE",3,.9f,.94f,1);label(470,325,std::to_string(score),3,.9f,.94f,1);label(310,395,"SPACE TO PLAY AGAIN",2,.4f,.96f,.9f);}}
}
void draw(){
    buildFrame();pUseProgram(program);pUniform2f(resolution,W,H);pBindVertexArray(vao);pBindBuffer(ARRAY_BUFFER,vbo);
    pBufferData(ARRAY_BUFFER,static_cast<std::ptrdiff_t>(vertices.size()*sizeof(Vertex)),vertices.data(),DYNAMIC_DRAW);
    
    pDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()));SwapBuffers(dc);
}

void update(float dt){
    if(screen!=1)return;
    float speed=300*dt;
    if(keys[VK_LEFT]||keys['A'])px-=speed;
    if(keys[VK_RIGHT]||keys['D'])px+=speed;
    if(keys[VK_UP]||keys['W'])py-=speed;
    if(keys[VK_DOWN]||keys['S'])py+=speed;
    px=std::clamp(px,42.0f,float(W-42));py=std::clamp(py,float(TOP+26),float(BOTTOM-25));shield=std::max(0.0f,shield-dt);
    if(++spawn>=34){spawn=0;std::uniform_int_distribution<int>x(45,W-45),kind(0,99);std::uniform_real_distribution<float>fall(2.2f,4.2f);bool pixel=kind(rng)<65;objects.push_back({float(x(rng)),TOP+10.0f,fall(rng),pixel?13.0f:19.0f,pixel});}
    for(auto i=objects.begin();i!=objects.end();){i->y+=i->speed;float dx=i->x-px,dy=i->y-py,r=i->pixel?25:29;
        if(dx*dx+dy*dy<r*r){if(i->pixel)score+=10;else if(shield==0){--lives;shield=1;}i=objects.erase(i);}
        else if(i->y>BOTTOM+25)i=objects.erase(i);else ++i;}
    if(lives==0)screen=2;
}

LRESULT CALLBACK procedure(HWND hwnd,UINT msg,WPARAM key,LPARAM data){
    switch(msg){case WM_KEYDOWN:if(key<256)keys[key]=true;if((key==VK_SPACE||key==VK_RETURN)&&screen!=1)reset();if(key==VK_ESCAPE)DestroyWindow(hwnd);return 0;
    case WM_KEYUP:if(key<256)keys[key]=false;return 0;case WM_KILLFOCUS:std::fill(keys,keys+256,false);return 0;
    case WM_PAINT:{PAINTSTRUCT p;BeginPaint(hwnd,&p);EndPaint(hwnd,&p);return 0;}case WM_CLOSE:DestroyWindow(hwnd);return 0;
    case WM_DESTROY:PostQuitMessage(0);return 0;default:return DefWindowProcW(hwnd,msg,key,data);}
}

bool initOpenGL(){
    PIXELFORMATDESCRIPTOR p{};p.nSize=sizeof(p);p.nVersion=1;p.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;p.iPixelType=PFD_TYPE_RGBA;p.cColorBits=32;p.cAlphaBits=8;p.iLayerType=PFD_MAIN_PLANE;
    int format=ChoosePixelFormat(dc,&p);if(!format||!SetPixelFormat(dc,format,&p)){glError=L"Не удалось настроить формат графического окна.";return false;}
    HGLRC old=wglCreateContext(dc);if(!old||!wglMakeCurrent(dc,old)){glError=L"Драйвер не смог создать OpenGL-контекст.";return false;}
    auto create=loadGL<CreateContext>("wglCreateContextAttribsARB");if(!create){glError=L"Драйвер не поддерживает создание современного OpenGL-контекста.";return false;}
    const int versions[][2]={{4,6},{4,5},{4,4},{4,3},{4,2},{4,1},{4,0},{3,3}};
    for(const auto& version:versions){
        const int attrs[]={0x2091,version[0],0x2092,version[1],0x9126,1,0};
        context=create(dc,nullptr,attrs);
        if(context)break;
    }
    wglMakeCurrent(nullptr,nullptr);wglDeleteContext(old);
    if(!context){glError=L"OpenGL 3.3 или новее не поддерживается видеодрайвером.";return false;}
    if(!wglMakeCurrent(dc,context)){glError=L"Не удалось активировать OpenGL-контекст.";return false;}
    if(!loadFunctions()){glError=L"Не удалось загрузить OpenGL-функции VAO/VBO/шейдеров.";return false;}
    const char* vs="#version 330 core\nlayout(location=0)in vec2 p;layout(location=1)in vec3 c;out vec3 v;uniform vec2 size;void main(){vec2 q=p/size*2.-1.;q.y=-q.y;gl_Position=vec4(q,0,1);v=c;}";
    const char* fs="#version 330 core\nin vec3 v;out vec4 f;void main(){f=vec4(v,1);}";
    GLuint v=makeShader(VERTEX_SHADER,vs),f=makeShader(FRAGMENT_SHADER,fs);if(!v||!f){glError=L"Не удалось скомпилировать GLSL-шейдер.";return false;}
    program=pCreateProgram();pAttachShader(program,v);pAttachShader(program,f);pLinkProgram(program);pDeleteShader(v);pDeleteShader(f);GLint ok=0;pGetProgramiv(program,LINK_STATUS,&ok);if(!ok){glError=L"Не удалось связать GLSL-шейдеры в программу.";return false;}
    resolution=pGetUniform(program,"size");pGenVertexArrays(1,&vao);pBindVertexArray(vao);pGenBuffers(1,&vbo);pBindBuffer(ARRAY_BUFFER,vbo);
    pEnableAttrib(0);pAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),nullptr);
    pEnableAttrib(1);pAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(2*sizeof(float)));
    if(const auto* version=reinterpret_cast<const char*>(glGetString(GL_VERSION))){
        std::wstring title=L"Pixel Patrol | OpenGL ";
        while(*version)title+=static_cast<unsigned char>(*version++);
        SetWindowTextW(window,title.c_str());
    }
    glViewport(0,0,W,H);return true;
}
}

int WINAPI WinMain(HINSTANCE app,HINSTANCE,LPSTR,int show){
    const wchar_t cls[]=L"PixelPatrolGL";WNDCLASSW wc{};wc.style=CS_OWNDC;wc.lpfnWndProc=procedure;wc.hInstance=app;wc.hCursor=LoadCursorW(nullptr,MAKEINTRESOURCEW(32512));wc.lpszClassName=cls;
    if(!RegisterClassW(&wc)){MessageBoxW(nullptr,L"Windows не зарегистрировал игровой класс окна.",L"Запуск игры",MB_ICONERROR);return 10;}
    DWORD style=WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;RECT r{0,0,W,H};AdjustWindowRect(&r,style,FALSE);
    window=CreateWindowExW(0,cls,L"Pixel Patrol — OpenGL",style,CW_USEDEFAULT,CW_USEDEFAULT,r.right-r.left,r.bottom-r.top,nullptr,nullptr,app,nullptr);
    if(!window){MessageBoxW(nullptr,L"Windows не смог открыть окно игры.",L"Запуск игры",MB_ICONERROR);return 11;}
    dc=GetDC(window);if(!initOpenGL()){MessageBoxW(window,glError,L"Ошибка OpenGL",MB_ICONERROR);return 12;}
    ShowWindow(window,show?show:SW_SHOW);previous=std::chrono::steady_clock::now();MSG msg{};bool run=true;
    while(run){while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){if(msg.message==WM_QUIT){run=false;break;}TranslateMessage(&msg);DispatchMessageW(&msg);}if(!run)break;
        auto now=std::chrono::steady_clock::now();float dt=std::min(std::chrono::duration<float>(now-previous).count(),.05f);previous=now;update(dt);draw();Sleep(16);}
    if(context)wglMakeCurrent(nullptr,nullptr);
    if(program)pDeleteProgram(program);
    if(vbo)pDeleteBuffers(1,&vbo);
    if(vao)pDeleteVertexArrays(1,&vao);
    if(context)wglDeleteContext(context);
    ReleaseDC(window,dc);UnregisterClassW(cls,app);
    return int(msg.wParam);
}
