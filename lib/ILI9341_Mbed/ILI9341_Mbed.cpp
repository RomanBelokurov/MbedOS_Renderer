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


#include "ILI9341_Mbed.h"
#include "mbed.h"


ILI9341_Mbed::ILI9341_Mbed(SPI* spiInterface, DigitalOut* cs, DigitalOut* reset, DigitalOut* dc)
{
    _spi = spiInterface;
    _cs = cs;
    _dc = dc;
    _reset = reset;

    _orientation = 0;
    _char_x = 0;
    _char_y = 0;

    // setup spi
    _cs->write(1);
	_spi->format(8, 3);
    _spi->frequency(10000000);          // 10 Mhz SPI clock
    _cs->write(1);
    
    tftReset();
}

void ILI9341_Mbed::setOrientation(unsigned int orientation)
{
    _orientation = orientation;
    writeCmd(0x36);
    switch (_orientation) {
        case 0:
            _spi->write(0x48);
            break;
        case 1:
            _spi->write(0x28);
            break;
        case 2:
            _spi->write(0x88);
            break;
        case 3:
            _spi->write(0xE8);
            break;
    }
    _cs->write(1); 
    window(0, 0, getWidth(),  getHeight());
}

int ILI9341_Mbed::getWidth()
{
    if (_orientation == 0 || _orientation == 2) return TFT_WIDTH;
    else return TFT_HEIGHT;
}

int ILI9341_Mbed::getHeight()
{
    if (_orientation == 0 || _orientation == 2) return TFT_HEIGHT;
    else return TFT_WIDTH;
}

void ILI9341_Mbed::putPixel(int x, int y, int color)
{
    writeCmd(0x2A);
    _spi->write(x >> 8);
    _spi->write(x);
    _cs->write(1);

    writeCmd(0x2B);
    _spi->write(y >> 8);
    _spi->write(y);
    _cs->write(1);

    writeCmd(0x2C);
    _spi->format(16,3);
    _spi->write(color);
    _spi->format(8,3);
    _cs->write(1);
}

void ILI9341_Mbed::rect(int x0, int y0, int x1, int y1, int color)
{
    if (x1 > x0) hline(x0,x1,y0,color);
    else  hline(x1,x0,y0,color);

    if (y1 > y0) vline(x0,y0,y1,color);
    else vline(x0,y1,y0,color);

    if (x1 > x0) hline(x0,x1,y1,color);
    else  hline(x1,x0,y1,color);

    if (y1 > y0) vline(x1,y0,y1,color);
    else vline(x1,y1,y0,color);

    return;
}

void ILI9341_Mbed::fillRect(int x0, int y0, int x1, int y1, int color)
{

    int h = y1 - y0 + 1;
    int w = x1 - x0 + 1;
    int pixel = h * w;
    window(x0, y0, w, h);
    writeCmd(0x2C);
    
    _spi->format(16, 3);

    for (int p=0; p<pixel; p++) {
        _spi->write(color);
    }
    _spi->format(8, 3);
    
    _cs->write(1);
    window(0, 0, getWidth(),  getHeight());
    return;
}

void ILI9341_Mbed::circle(int x0, int y0, int r, int color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        putPixel(x0-x, y0+y,color);
        putPixel(x0+x, y0+y,color);
        putPixel(x0+x, y0-y,color);
        putPixel(x0-x, y0-y,color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}

void ILI9341_Mbed::fillCircle(int x0, int y0, int r, int color)
{
    int x = -r, y = 0, err = 2-2*r, e2;
    do {
        vline(x0-x, y0-y, y0+y, color);
        vline(x0+x, y0-y, y0+y, color);
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    } while (x <= 0);
}

void ILI9341_Mbed::hline(int x0, int x1, int y, int color)
{
    int w;
    w = x1 - x0 + 1;
    window(x0,y,w,1);
    writeCmd(0x2C);  // send pixel
    
    _spi->format(16,3);                            // switch to 16 bit Mode 3
    int j;
    for (j=0; j<w; j++) {
        _spi->write(color);
    }
    _spi->format(8,3);
    
    _cs->write(1);
    window(0, 0, getWidth(),  getHeight());
    return;
}

void ILI9341_Mbed::vline(int x, int y0, int y1, int color)
{
    int h;
    h = y1 - y0 + 1;
    window(x,y0,1,h);
    writeCmd(0x2C);  // send pixel
    
    _spi->format(16,3);                            // switch to 16 bit Mode 3
    for (int y=0; y<h; y++) {
        _spi->write(color);
    }
    _spi->format(8,3);    
    _cs->write(1);

    window(0, 0, getWidth(),  getHeight());
    return;
}

void ILI9341_Mbed::line(int x0, int y0, int x1, int y1, int color)
{
    //WindowMax();
    int   dx = 0, dy = 0;
    int   dx_sym = 0, dy_sym = 0;
    int   dx_x2 = 0, dy_x2 = 0;
    int   di = 0;

    dx = x1-x0;
    dy = y1-y0;

    if (dx == 0) {        /* vertical line */
        if (y1 > y0) vline(x0,y0,y1,color);
        else vline(x0,y1,y0,color);
        return;
    }

    if (dx > 0) {
        dx_sym = 1;
    } else {
        dx_sym = -1;
    }
    if (dy == 0) {        /* horizontal line */
        if (x1 > x0) hline(x0,x1,y0,color);
        else  hline(x1,x0,y0,color);
        return;
    }

    if (dy > 0) {
        dy_sym = 1;
    } else {
        dy_sym = -1;
    }

    dx = dx_sym*dx;
    dy = dy_sym*dy;

    dx_x2 = dx*2;
    dy_x2 = dy*2;

    if (dx >= dy) {
        di = dy_x2 - dx;
        while (x0 != x1) {

            putPixel(x0, y0, color);
            x0 += dx_sym;
            if (di<0) {
                di += dy_x2;
            } else {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        putPixel(x0, y0, color);
    } else {
        di = dx_x2 - dy;
        while (y0 != y1) {
            putPixel(x0, y0, color);
            y0 += dy_sym;
            if (di < 0) {
                di += dx_x2;
            } else {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        putPixel(x0, y0, color);
    }
    return;
}

void ILI9341_Mbed::tftReset()
{
    _cs->write(1);
    _dc->write(1);
    _reset->write(0);

    wait_us(50);
    _reset->write(1);
	thread_sleep_for(5);
     
    writeCmd(0x01);
    
	thread_sleep_for(5);
    writeCmd(0x28);


    /* Start Initial Sequence ----------------------------------------------------*/
     writeCmd(0xCF);                     
     _spi->write(0x00);
     _spi->write(0x83);
     _spi->write(0x30);
     _cs->write(1);
     
     writeCmd(0xED);                     
     _spi->write(0x64);
     _spi->write(0x03);
     _spi->write(0x12);
     _spi->write(0x81);
     _cs->write(1);
     
     writeCmd(0xE8);                     
     _spi->write(0x85);
     _spi->write(0x01);
     _spi->write(0x79);
     _cs->write(1);
     
     writeCmd(0xCB);                     
     _spi->write(0x39);
     _spi->write(0x2C);
     _spi->write(0x00);
     _spi->write(0x34);
     _spi->write(0x02);
     _cs->write(1);
           
     writeCmd(0xF7);                     
     _spi->write(0x20);
     _cs->write(1);
           
     writeCmd(0xEA);                     
     _spi->write(0x00);
     _spi->write(0x00);
     _cs->write(1);
     
     writeCmd(0xC0);                     // POWER_CONTROL_1
     _spi->write(0x26);
     _cs->write(1);
 
     writeCmd(0xC1);                     // POWER_CONTROL_2
     _spi->write(0x11);
     _cs->write(1);
     
     writeCmd(0xC5);                     // VCOM_CONTROL_1
     _spi->write(0x35);
     _spi->write(0x3E);
     _cs->write(1);
     
     writeCmd(0xC7);                     // VCOM_CONTROL_2
     _spi->write(0xBE);
     _cs->write(1);
     
     writeCmd(0x36);                     // MEMORY_ACCESS_CONTROL
     _spi->write(0x48);
     _cs->write(1);
     
     writeCmd(0x3A);                     // COLMOD_PIXEL_FORMAT_SET
     _spi->write(0x55);                 // 16 bit pixel 
     _cs->write(1);
     
     writeCmd(0xB1);                     // Frame Rate
     _spi->write(0x00);
     _spi->write(0x1B);               
     _cs->write(1);
     
     writeCmd(0xF2);                     // Gamma Function Disable
     _spi->write(0x08);
     _cs->write(1);
     
     writeCmd(0x26);                     
     _spi->write(0x01);                 // gamma set for curve 01/2/04/08
     _cs->write(1);
     
     writeCmd(0xE0);                     // positive gamma correction
     _spi->write(0x1F); 
     _spi->write(0x1A); 
     _spi->write(0x18); 
     _spi->write(0x0A); 
     _spi->write(0x0F); 
     _spi->write(0x06); 
     _spi->write(0x45); 
     _spi->write(0x87); 
     _spi->write(0x32); 
     _spi->write(0x0A); 
     _spi->write(0x07); 
     _spi->write(0x02); 
     _spi->write(0x07);
     _spi->write(0x05); 
     _spi->write(0x00);
     _cs->write(1);
     
     writeCmd(0xE1);                     // negativ gamma correction
     _spi->write(0x00); 
     _spi->write(0x25); 
     _spi->write(0x27); 
     _spi->write(0x05); 
     _spi->write(0x10); 
     _spi->write(0x09); 
     _spi->write(0x3A); 
     _spi->write(0x78); 
     _spi->write(0x4D); 
     _spi->write(0x05); 
     _spi->write(0x18); 
     _spi->write(0x0D); 
     _spi->write(0x38);
     _spi->write(0x3A); 
     _spi->write(0x1F);
     _cs->write(1);
     
     
     window(0, 0, getWidth(),  getHeight());
     
      
     writeCmd(0xB7);                       // entry mode
     _spi->write(0x07);
     _cs->write(1);
     
     writeCmd(0xB6);                       // display function control
     _spi->write(0x0A);
     _spi->write(0x82);
     _spi->write(0x27);
     _spi->write(0x00);
     _cs->write(1);
     
     writeCmd(0x11);                     // sleep out
     _cs->write(1);
     
	 thread_sleep_for(100);
     writeCmd(0x29);                     // display on
     _cs->write(1);
     
	 thread_sleep_for(100);
}

void ILI9341_Mbed::writeCmd(unsigned char cmd)
{
    _dc->write(0);
    _cs->write(0);
    _spi->write(cmd);
    _dc->write(1);
}

void ILI9341_Mbed::window(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    writeCmd(0x2A);
    _spi->write(x >> 8);
    _spi->write(x);
    _spi->write((x+w-1) >> 8);
    _spi->write(x+w-1);
    _cs->write(1);

    writeCmd(0x2B);
    _spi->write(y >> 8);
    _spi->write(y);
    _spi->write((y+h-1) >> 8);
    _spi->write(y+h-1);
    _cs->write(1);
}



void ILI9341_Mbed::locate(int x, int y)
{
    _char_x = x;
    _char_y = y;
}

void ILI9341_Mbed::set_font(unsigned char* f)
{
    font = f;
}

void ILI9341_Mbed::character(int x, int y, int c)
{
    unsigned int hor,vert,offset,bpl,j,i,b;
    unsigned char* zeichen;
    unsigned char z,w;

    if ((c < 31) || (c > 127)) return;   // test char range

    // read font parameter from start of array
    offset = font[0];                    // bytes / char
    hor = font[1];                       // get hor size of font
    vert = font[2];                      // get vert size of font
    bpl = font[3];                       // bytes per line

    if (_char_x + hor > getWidth()) {
        _char_x = 0;
        _char_y = _char_y + vert;
        if (_char_y >= getHeight() - font[2]) {
            _char_y = 0;
        }
    }
    window(_char_x, _char_y, hor, vert); // char box
    writeCmd(0x2C);  // send pixel

    _spi->format(16,3);   

    zeichen = &font[((c -32) * offset) + 4]; // start of char bitmap
    w = zeichen[0];                          // width of actual char
     for (j=0; j<vert; j++) {  //  vert line
        for (i=0; i<hor; i++) {   //  horz line
            z =  zeichen[bpl * i + ((j & 0xF8) >> 3)+1];
            b = 1 << (j & 0x07);
            if (( z & b ) == 0x00) {
                _spi->write(Black);
            } else {
                _spi->write(White);                
            }
        }
    }
    _cs->write(1);
    _spi->format(8,3);

    window (0, 0, getWidth(),  getHeight());
    
    if ((w + 2) < hor) 
        _char_x += w + 2;
    else 
        _char_x += hor;
}
