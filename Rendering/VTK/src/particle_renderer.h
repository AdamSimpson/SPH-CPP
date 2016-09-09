/*
The MIT License (MIT)

Copyright (c) 2016 Adam Simpson

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef VTK_SRC_PARTICLE_RENDERER_H_
#define VTK_SRC_PARTICLE_RENDERER_H_

#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkOpenGLSphereMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkWindowToImageFilter.h>
#include <vtkOggTheoraWriter.h>
#include <vtkOBJReader.h>

#include "parameters.h"
#include "boost/mpi.hpp"
#include <vector>

class VtkParticleRenderer {
  public:
    VtkParticleRenderer() : points{vtkSmartPointer<vtkPoints>::New()},
                            radii{vtkSmartPointer<vtkFloatArray>::New()},
                            colors{vtkSmartPointer<vtkUnsignedCharArray>::New()},
                            obstacle_reader{vtkSmartPointer<vtkOBJReader>::New()},
                            fluid_polydata{vtkSmartPointer<vtkPolyData>::New()},
                            fluid_mapper{vtkSmartPointer<vtkOpenGLSphereMapper>::New()},
                            obstacle_mapper{vtkSmartPointer<vtkPolyDataMapper>::New()},
                            fluid_actor{vtkSmartPointer<vtkActor>::New()},
                            obstacle_actor{vtkSmartPointer<vtkActor>::New()},
                            renderer{vtkSmartPointer<vtkRenderer>::New()},
                            render_window{vtkSmartPointer<vtkRenderWindow>::New()},
                            camera{vtkSmartPointer<vtkCamera>::New()},
                            ogg_writer{vtkSmartPointer<vtkOggTheoraWriter>::New()},
                            window_to_image_filter{vtkSmartPointer<vtkWindowToImageFilter>::New()},
                            params{Parameters{"./post-config.ini"}}{};
  void PrepareBuffers(std::size_t step, const boost::mpi::communicator& comm);
    void PreparePolyData();
    void PrepareScene();
    void RenderScene();
    void BeginAnimation();
    void EndAnimation();
    std::vector<std::string> GetFiles();
    void ReadParameters();

  private:
    Parameters params;
    vtkSmartPointer<vtkPoints> points;
    vtkSmartPointer<vtkFloatArray> radii;
    vtkSmartPointer<vtkUnsignedCharArray> colors;
    vtkSmartPointer<vtkPolyData> fluid_polydata;
    vtkSmartPointer<vtkOBJReader> obstacle_reader;
    vtkSmartPointer<vtkOpenGLSphereMapper> fluid_mapper;
    vtkSmartPointer<vtkPolyDataMapper> obstacle_mapper;
    vtkSmartPointer<vtkActor> fluid_actor;
    vtkSmartPointer<vtkActor> obstacle_actor;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkRenderWindow> render_window;
    vtkSmartPointer<vtkCamera> camera;
    vtkSmartPointer<vtkOggTheoraWriter> ogg_writer;
    vtkSmartPointer<vtkWindowToImageFilter> window_to_image_filter;
};

#endif
