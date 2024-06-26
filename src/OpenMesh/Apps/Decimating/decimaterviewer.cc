/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2025, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */



#ifdef _MSC_VER
#  pragma warning(disable: 4267 4311)
#endif

#include <iostream>
#include <fstream>
#include <OpenMesh/Tools/Utils/getopt.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <QOpenGLContext>

#include "DecimaterViewerWidget.hh"

void usage_and_exit(int xcode);


int main(int argc, char **argv)
{
#if defined(OM_USE_OSG) && OM_USE_OSG
  osg::osgInit(argc, argv);
#endif
  
  // OpenGL check
  QApplication app(argc,argv);

#if QT_VERSION_MAJOR < 6
  if ( !QGLFormat::hasOpenGL() ) {
#else
  if ( QOpenGLContext::openGLModuleType() != QOpenGLContext::LibGL ) {
#endif
    QString msg = "System has no OpenGL support!";
    QMessageBox::critical( nullptr, "OpenGL", msg + argv[1] );
    return -1;
  }


  int c;
  OpenMesh::IO::Options opt;
  
  while ( (c=getopt(argc,argv,"s"))!=-1 )
  {
     switch(c)
     {
       case 's': opt += OpenMesh::IO::Options::Swap; break;
       case 'h':
          usage_and_exit(0);
          break;
       default:
          usage_and_exit(1);
     }
  }
  // create widget
  DecimaterViewerWidget w(0);  
//  app.setMainWidget(&w);

  w.resize(400, 400);
  w.show(); 

  // load scene
  if ( optind < argc )  
  {
     if ( ! w.open_mesh(argv[optind], opt) )
     {
        QString msg = "Cannot read mesh from file:\n '";
        msg += argv[optind];
        msg += "'";
        QMessageBox::critical( nullptr, w.windowTitle(), msg );
        return 1;
     }
  }

  if ( ++optind < argc )
  {
     if ( ! w.open_texture( argv[optind] ) )
     {
         QString msg = "Cannot load texture image from file:\n '";
        msg += argv[optind];
        msg += "'\n\nPossible reasons:\n";
        msg += "- Mesh file didn't provide texture coordinates\n";
        msg += "- Texture file does not exist\n";
        msg += "- Texture file is not accessible.\n";
        QMessageBox::warning( nullptr, w.windowTitle(), msg );
     }
  }

  return app.exec();
}

void usage_and_exit(int xcode)
{
   std::cout << "Usage: DecimaterGui [-s] [mesh] [texture]\n" << std::endl;
   std::cout << "Options:\n"
             << "  -s\n"
             << "    Reverse byte order, when reading binary files.\n"
             << "    Press 'h' when the application is running for more options.\n"
             << std::endl;
   exit(xcode);
}
