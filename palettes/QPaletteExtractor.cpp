#include "QPaletteExtractor.h"

#include <QImage>
#include <QFileInfo>
#include <QDebug>

enum  { C_R, C_G, C_B };
struct myrgb_t
{
  uchar t[3];
};
inline unsigned int cast(const myrgb_t& clr){  return clr.t[C_R] << 16 | clr.t[C_G] << 8 | clr.t[C_B]; }

inline myrgb_t convert(const uchar* rawdata)
{
  myrgb_t result;
//      for (int b=0; b<bpc; b++)
//        rd[b] = rawdata[b];
//      rd[3] = 0;
  for (int b=0; b<3; b++)
    result.t[b] = rawdata[b];
//    result.t[b] = rawdata[b];
  return result;
}

inline myrgb_t complex(const myrgb_t* rawdata, int count)
{
  unsigned int t[3] = { 0,0,0 };
  for (int i=0; i<count; i++)
    for (int c=0; c<3; c++)
      t[c] += rawdata[i].t[c];
  myrgb_t result = { uchar(t[0]/count), uchar(t[1]/count), uchar(t[2]/count) };
  return result;
}

pexresult_t paletteExtractFromImage(const char* fname, int flags, IMAGE_DIRECTION imd, const char* resultName)
{
  pexresult_t   result;
  result.success = false;
  QFileInfo fi(fname);
  if (!fi.exists())
  {
    qDebug()<<"PaletteExtract: No image found!"<<fname;
    return result;
  }
  QImage* pImage = new QImage(QString(fname));
  const int w = pImage->width(), h = pImage->height();
  int bpl = pImage->bytesPerLine(), depth = pImage->depth();
  if (depth != 24 && depth != 32)
  {
    qDebug()<<"PaletteExtract: only 24bit and 32bit depth support"<<fname;
    return result;
  }
  qDebug()<<"Parsing image: "<<QSize(w,h)<<"; bytesPerLine = "<<bpl<<"; depth: "<<depth<<"; format: "<<pImage->format();
  
  if (imd == IMD_NATIVE)
    imd = w >= h? IMD_HORIZONTAL : IMD_VERTICAL;
  const int tech_count = imd == IMD_HORIZONTAL? w : h;
  const int bpc = depth == 24? 3 : 4;
  
  myrgb_t* tech_clrcodes = new myrgb_t[tech_count];
  if (imd == IMD_HORIZONTAL)
  {
    const uchar* data = pImage->scanLine(0);
    for (int i=0; i<tech_count; i++)
      tech_clrcodes[i] = convert(&data[i*bpc]);
  }
  else if (imd == IMD_VERTICAL)
  {
    for (int i=0; i<tech_count; i++)
      tech_clrcodes[i] = convert(pImage->scanLine(i));
  }
  
  myrgb_t* clrcodes = tech_clrcodes;
  int count = tech_count;
  if (flags & PEF_REMOVEFIRSTCOLOR)
  {
    clrcodes = &tech_clrcodes[1];
    count--;
  }
  if (flags & PEF_REMOVELASTCOLOR)
    count--;
  
  if (flags & PEF_NOMAXLIMIT)
    ;
  else
  {
    if (flags & PEF_CUTTER_GROUP)
    {
      if (count > MAX_PALETTE_COLORS_COUNT)
      {
        int total = count - 2;
        int groupsize = 1;
        while (total / groupsize > MAX_PALETTE_COLORS_COUNT)
          groupsize++;
        int groupscount = total/groupsize;
        qDebug()<<groupsize<<groupscount;
        for (int i=0; i<groupscount; i++)
          clrcodes[1 + i] = complex(&clrcodes[1 + i*groupsize], groupsize);
        clrcodes[1 + groupscount] = clrcodes[count-1];
        count = 1 + groupscount + 1;   
      }
    }
    else
    {
      while (count > MAX_PALETTE_COLORS_COUNT)
      {
//        int diff = count - MAX_PALETTE_COLORS_COUNT;
  //    if (float(diff) / MAX_PALETTE_COLORS_COUNT > 1.0f)
  //    {
        for (int i=1; i<count/2; i++)
        {
          clrcodes[i] = clrcodes[2*i];
        }
        clrcodes[count/2] = clrcodes[count-1];
        count = count/2 + 1;
  //    }
  //    else
  //    {
  //    }
      }
    }
  }
    
  const int CPL = 8;
  const int TABS2 = 16;
  QString clrcodesString;
  for (int i=0; i<count; i++)
  {
    if (i % CPL == 0)
       clrcodesString += QString(TABS2, ' ');
    clrcodesString += QString(i+1 < count? "0x%1, " : "0x%1\n").arg(cast(clrcodes[i]), 8, 16, QChar('0'));
    if ((i+1) % CPL == 0)
      clrcodesString += QString("\n");
  }
  delete []tech_clrcodes;
  
  result.success = true;
  result.countColors = count;
  result.absolutePath = fi.absolutePath();
  result.paletteName = resultName == nullptr? fi.baseName() : resultName;
  result.paletteHeaderText = QString(
      "#ifndef PALETTE_%1\n"
      "#define PALETTE_%1\n"
      "\n"
      "#include \"../bsipalette.h\""
      "\n"
      "const unsigned int colors_%2[] = {\n" + clrcodesString +
      "};\n"
      "const PaletteConstFWD<sizeof(colors_%2) / sizeof(unsigned int)> palette_%2(colors_%2);\n"
      "const PaletteConstBWD<sizeof(colors_%2) / sizeof(unsigned int)> palette_%2_inv(colors_%2);\n"
      "\n" 
      "typedef PaletteConstFWD<sizeof(colors_%2) / sizeof(unsigned int)> palette_%2_t;\n"
      "\n"
    "#endif // PALETTE_%1\n").arg(result.paletteName.toUpper()).arg(result.paletteName)
  ;
  return result;
}

pexresult_t paletteExtractFromImage(const QString& fname, int flags, IMAGE_DIRECTION imd, const char* resultName)
{
  return paletteExtractFromImage(fname.toStdString().c_str(), flags, imd, resultName);
}

bool savePaletteExtract(const pexresult_t& pex)
{
  if (pex.success == false)
    return false;
  QFile f(pex.absolutePath + "/" + pex.paletteName + ".h");
  if (f.open(QFile::WriteOnly) == false)
    return false;
  
  f.write(pex.paletteHeaderText.toUtf8());
  f.close();

  return true;
}
