/* mbed library for 240*320 pixel display TFT based on ILI9341 LCD Controller
 * for mbed os 6. 
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef ILI9341_MBED_H
#define ILI9341_MBED_H

#include "mbed.h"

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

#define RGB(r,g,b)  (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3))

#define Black           0x0000      /*   0,   0,   0 */
#define Navy            0x000F      /*   0,   0, 128 */
#define DarkGreen       0x03E0      /*   0, 128,   0 */
#define DarkCyan        0x03EF      /*   0, 128, 128 */
#define Maroon          0x7800      /* 128,   0,   0 */
#define Purple          0x780F      /* 128,   0, 128 */
#define Olive           0x7BE0      /* 128, 128,   0 */
#define LightGrey       0xC618      /* 192, 192, 192 */
#define DarkGrey        0x7BEF      /* 128, 128, 128 */
#define Blue            0x001F      /*   0,   0, 255 */
#define Green           0x07E0      /*   0, 255,   0 */
#define Cyan            0x07FF      /*   0, 255, 255 */
#define Red             0xF800      /* 255,   0,   0 */
#define Magenta         0xF81F      /* 255,   0, 255 */
#define Yellow          0xFFE0      /* 255, 255,   0 */
#define White           0xFFFF      /* 255, 255, 255 */
#define Orange          0xFD20      /* 255, 165,   0 */
#define GreenYellow     0xAFE5      /* 173, 255,  47 */



class ILI9341_Mbed
{
    private:
        SPI* _spi;
        DigitalOut* _cs;
        DigitalOut* _dc;
        DigitalOut* _reset;

    private:
        unsigned int _orientation;
        unsigned int _width;
        unsigned int _height;

        unsigned int _char_x;
        unsigned int _char_y;

        unsigned char* font;

    public:
        ILI9341_Mbed(SPI* spiInterface, DigitalOut* cs, DigitalOut* reset, DigitalOut* dc);

        void setOrientation(unsigned int orientation);
        int getWidth();
        int getHeight();
    
    public:
        void putPixel(int x, int y, int color);

        void rect(int x0, int y0, int x1, int y1, int color);
        void fillRect(int x0, int y0, int x1, int y1, int color);
        
        void circle(int x0, int y0, int r, int color);
        void fillCircle(int x0, int y0, int r, int color);

        void line(int x0, int y0, int x1, int y1, int color);

        void locate(int x, int y);
        void set_font(unsigned char* f);
        void character(int x, int y, int c);
    
    // private helpers
    private:
        void vline(int x, int y0, int y1, int color);
        void hline(int x0, int x1, int y, int color);
    
    // private driver methods
    private:
        void tftReset();
        void writeCmd(unsigned char cmd);
        void window(unsigned int x, unsigned int y, unsigned int w, unsigned int h);        
};

#endif