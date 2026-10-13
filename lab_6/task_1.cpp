
#include <GL/freeglut.h>
#include <cmath>

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 600;

float triangleX = 1.13f;
float triangleY = -0.45f;
const float triangleSpeed = -0.55f;

float ballX = 0.0f;
float ballY = -0.66f;
const float ballRadius = 0.09f;

float velocityY = 2.8f;
const float gravity = 2.5f;
const float floorY = -0.75f;

int previousTime = 0;

void drawCircle(float x, float y, float radius)
{
    const int segments = 100;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);

    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * 3.14159265f * i / segments;
        glVertex2f(
            x + radius * cosf(angle),
            y + radius * sinf(angle)
        );
    }

    glEnd();
}

void drawTriangle(float x, float y)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 0.13f);
    glVertex2f(x - 0.13f, y - 0.10f);
    glVertex2f(x + 0.13f, y - 0.10f);
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_LINES);
    glVertex2f(-1.0f, floorY);
    glVertex2f(1.0f, floorY);
    glEnd();

    glColor3f(0.10f, 0.35f, 0.95f);
    drawTriangle(triangleX, triangleY);

    glColor3f(0.95f, 0.15f, 0.12f);
    drawCircle(ballX, ballY, ballRadius);

    glutSwapBuffers();
}

void update()
{
    int currentTime = glutGet(GLUT_ELAPSED_TIME);

    float dt = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    if (dt > 0.03f)
        dt = 0.03f;

    triangleX += triangleSpeed * dt;

    if (triangleX + 0.13f < -1.0f)
        triangleX = 1.13f;

    velocityY -= gravity * dt;
    ballY += velocityY * dt;

    if (ballY - ballRadius < floorY)
    {
        ballY = floorY + ballRadius;
        velocityY = -velocityY;
    }

    glutPostRedisplay();
}

void init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);

    glutCreateWindow("Computer Graphics");

    init();

    previousTime = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutIdleFunc(update);

    glutMainLoop();

    return 0;
}
