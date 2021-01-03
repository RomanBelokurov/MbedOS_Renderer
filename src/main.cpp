/* Embedded GameDev. Software rendering for 240*320 pixel display TFT based on ILI9341 LCD Controller
 * mbed os 6.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <mbed.h>
#include <ILI9341_Mbed.h>
#include <Arial12x12.h>
#include <vector>

SPI spi(SPI_MOSI, SPI_MISO, SPI_SCK);

DigitalOut LCD_LED(PE_3);
DigitalOut LCD_CS(SPI_CS);
DigitalOut LCD_RESET(PE_2);
DigitalOut LCD_DC(PE_4);

unsigned char *font12x12 = (unsigned char *)Arial12x12;

ILI9341_Mbed lcd(&spi, &LCD_CS, &LCD_RESET, &LCD_DC);

template <class t>
struct vec3d
{
    t x, y, z;

    vec3d<t> &operator+(const vec3d<t> &a)
    {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
    }
    vec3d<t> &operator-(const vec3d<t> &a)
    {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        return *this;
    }
    vec3d<t> &operator*(const vec3d<t> &a)
    {
        x *= a.x;
        y *= a.y;
        z *= a.z;
        return *this;
    }
    vec3d<t> &operator*(float f)
    {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }
};

struct triangle
{
    vec3d<float> p[3];
};

struct mesh
{
    std::vector<triangle> tris;
};

struct mat4x4
{
    float m[4][4] = {0};
};

mesh meshCube;
mat4x4 matProj;
bool CreateCubeMesh(int screenWidth, int screenHeight)
{
    meshCube.tris = {

        // SOUTH
        {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f},

        // EAST
        {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

        // NORTH
        {1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},

        // WEST
        {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},

        // TOP
        {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},

        // BOTTOM
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},

    };

    // Projection Matrix
    float fNear = 0.1f;
    float fFar = 1000.0f;
    float fFov = 90.0f;
    float fAspectRatio = (float)screenHeight / (float)screenWidth;
    float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

    matProj.m[0][0] = fAspectRatio * fFovRad;
    matProj.m[1][1] = fFovRad;
    matProj.m[2][2] = fFar / (fFar - fNear);
    matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
    matProj.m[2][3] = 1.0f;
    matProj.m[3][3] = 0.0f;

    return true;
}

void MultiplyMatrixVector(vec3d<float> &i, vec3d<float> &o, mat4x4 &m)
{
    o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
    o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
    o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
    float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

    if (w != 0.0f)
    {
        o.x /= w;
        o.y /= w;
        o.z /= w;
    }
}

bool OnUpdate(float fTheta, int screenWidth, int screenHeight, int color)
{
    // Set up rotation matrices
    mat4x4 matRotZ, matRotX;

    // Rotation Z
    matRotZ.m[0][0] = cosf(fTheta);
    matRotZ.m[0][1] = sinf(fTheta);
    matRotZ.m[1][0] = -sinf(fTheta);
    matRotZ.m[1][1] = cosf(fTheta);
    matRotZ.m[2][2] = 1;
    matRotZ.m[3][3] = 1;

    // Rotation X
    matRotX.m[0][0] = 1;
    matRotX.m[1][1] = cosf(fTheta * 0.5f);
    matRotX.m[1][2] = sinf(fTheta * 0.5f);
    matRotX.m[2][1] = -sinf(fTheta * 0.5f);
    matRotX.m[2][2] = cosf(fTheta * 0.5f);
    matRotX.m[3][3] = 1;

    // Draw Triangles
    for (auto tri : meshCube.tris)
    {
        triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

        // Rotate in Z-Axis
        MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
        MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
        MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

        // Rotate in X-Axis
        MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
        MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
        MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

        // Offset into the screen
        triTranslated = triRotatedZX;
        triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
        triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
        triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

        // Project triangles from 3D --> 2D
        MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
        MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
        MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

        // Scale into view
        triProjected.p[0].x += 1.0f;
        triProjected.p[0].y += 1.0f;
        triProjected.p[1].x += 1.0f;
        triProjected.p[1].y += 1.0f;
        triProjected.p[2].x += 1.0f;
        triProjected.p[2].y += 1.0f;
        triProjected.p[0].x *= 0.5f * (float)screenWidth;
        triProjected.p[0].y *= 0.5f * (float)screenHeight;
        triProjected.p[1].x *= 0.5f * (float)screenWidth;
        triProjected.p[1].y *= 0.5f * (float)screenHeight;
        triProjected.p[2].x *= 0.5f * (float)screenWidth;
        triProjected.p[2].y *= 0.5f * (float)screenHeight;

        // Rasterize triangle
        lcd.line(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y, color);
        lcd.line(triProjected.p[1].x, triProjected.p[1].y, triProjected.p[2].x, triProjected.p[2].y, color);
        lcd.line(triProjected.p[2].x, triProjected.p[2].y, triProjected.p[0].x, triProjected.p[0].y, color);
    }

    return true;
}

int main()
{
    LCD_LED.write(1);

    lcd.setOrientation(1);
    lcd.fillRect(0, 0, lcd.getWidth() - 1, lcd.getHeight() - 1, Black);

    lcd.set_font(font12x12);
    lcd.locate(10, 10);

    int width = lcd.getWidth();
    int height = lcd.getHeight();

    CreateCubeMesh(width, height);

    float theta = 0.0f;
    while(true)
    {
        OnUpdate(theta, width, height, Green); // draw
        OnUpdate(theta, width, height, Black); // clear
        theta += 0.05f; // increase angle
    }
}