#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCylinderSource.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkPNGWriter.h>
#include <vtkRenderer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkSTLReader.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>
#include <vtkDataSetMapper.h>
#include <vtkFeatureEdges.h>
#include <vtkStripper.h>
#include <vtkCallbackCommand.h>
#include <array>

void CameraModifiedCallback(vtkObject* caller,
                            long unsigned int vtkNotUsed(eventId),
                            void* vtkNotUsed(clientData),
                            void* vtkNotUsed(callData))
{
  std::cout << caller->GetClassName() << " modified" << std::endl;
  vtkCamera* camera = static_cast<vtkCamera*>(caller);
  // Print the interesting stuff.
  std::cout << "  auto camera = renderer->GetActiveCamera();" << std::endl;
  std::cout << "  camera->SetPosition(" << camera->GetPosition()[0] << ", "
            << camera->GetPosition()[1] << ", " << camera->GetPosition()[2]
            << ");" << std::endl;
  std::cout << "  camera->SetFocalPoint(" << camera->GetFocalPoint()[0] << ", "
            << camera->GetFocalPoint()[1] << ", " << camera->GetFocalPoint()[2]
            << ");" << std::endl;
  std::cout << "  camera->SetViewUp(" << camera->GetViewUp()[0] << ", "
            << camera->GetViewUp()[1] << ", " << camera->GetViewUp()[2] << ");"
            << std::endl;
  std::cout << "  camera->SetDistance(" << camera->GetDistance() << ");"
            << std::endl;
  std::cout << "  camera->SetClippingRange(" << camera->GetClippingRange()[0]
            << ", " << camera->GetClippingRange()[1] << ");" << std::endl;
  std::cout << std::endl;
}

int main(int, char*[])
{
  vtkNew<vtkSTLReader> stlReader;
  stlReader->SetFileName("/mnt/chump/3dDruck/sechsschwaenzer/Dispenser_with_Funnel.stl");
  stlReader->Update();
  vtkNew<vtkPolyDataMapper> stlMapper;
  stlMapper->SetInputConnection(stlReader->GetOutputPort());


  // The actor is a grouping mechanism: besides the geometry (mapper), it
  // also has a property, transformation matrix, and/or texture map.
  // Here we set its color and rotate it around the X and Y axes.
  vtkNew<vtkActor> stlActor;
  stlActor->SetMapper(stlMapper);
  //stlActor->GetProperty()->SetColor(1,1,1);
  //stlActor->GetProperty()->SetAmbient(1);
  //stlActor->GetProperty()->SetDiffuse(0);
  //stlActor->GetProperty()->SetSpecular(0);
  stlActor->SetPosition(0,0,0);
  //stlActor->RotateX(30.0);
  //stlActor->RotateY(-45.0);


  vtkNew<vtkPlane> plane;
  plane->SetOrigin(0,0,10);
  plane->SetNormal(0, 0, -1);

  vtkNew<vtkClipPolyData> clipper;
  clipper->SetInputData(stlReader->GetOutput());
  clipper->SetClipFunction(plane);
  clipper->SetValue(0);
  clipper->Update();

  vtkNew<vtkDataSetMapper> clipMapper;
  clipMapper->SetInputData(clipper->GetOutput());

  vtkNew<vtkActor> clipActor;
  clipActor->SetMapper(clipMapper);
  clipActor->GetProperty()->SetColor(1,1,1);
  //clipActor->GetProperty()->SetAmbient(1);
  //clipActor->GetProperty()->SetDiffuse(0);
  //clipActor->GetProperty()->SetSpecular(0);
  clipActor->SetPosition(0,0,0);

  // Now extract feature edges
  vtkNew<vtkFeatureEdges> boundaryEdges;
  boundaryEdges->SetInputData(clipper->GetOutput());
  boundaryEdges->BoundaryEdgesOn();
  boundaryEdges->FeatureEdgesOff();
  boundaryEdges->NonManifoldEdgesOff();
  boundaryEdges->ManifoldEdgesOff();

  vtkNew<vtkStripper> boundaryStrips;
  boundaryStrips->SetInputConnection(boundaryEdges->GetOutputPort());
  boundaryStrips->Update();

  // Change the polylines into polygons
  vtkNew<vtkPolyData> boundaryPoly;
  boundaryPoly->SetPoints(boundaryStrips->GetOutput()->GetPoints());
  boundaryPoly->SetPolys(boundaryStrips->GetOutput()->GetLines());

  vtkNew<vtkPolyDataMapper> boundaryMapper;
  boundaryMapper->SetInputData(boundaryPoly);

  vtkNew<vtkActor> boundaryActor;
  boundaryActor->SetMapper(boundaryMapper);
  boundaryActor->GetProperty()->SetColor(1,0,0);
  //boundaryActor->GetProperty()->SetAmbient(1);
  //boundaryActor->GetProperty()->SetDiffuse(0);
  //boundaryActor->GetProperty()->SetSpecular(0);

  // The renderer generates the image
  // which is then displayed on the render window.
  // It can be thought of as a scene to which the actor is added
  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(clipActor);
  renderer->AddActor(boundaryActor);
  renderer->ResetCamera();
  renderer->GetActiveCamera()->SetParallelProjection(true);
  renderer->GetActiveCamera()->SetFocalPoint(0,0,0);

  vtkNew<vtkCallbackCommand> modifiedCallback;
  modifiedCallback->SetCallback(CameraModifiedCallback);
  renderer->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent,modifiedCallback);


  // The render window is the actual GUI window
  // that appears on the computer screen
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(1920, 1080);
  //renderWindow->SetOffScreenRendering(true);
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("Slicer Preview");


  /*
  for (double i=250; i<=1000; i+=0.1) {
      plane->SetOrigin(0,0,i);
      clipper->Update();
      boundaryStrips->Update();
      renderer->GetActiveCamera()->SetClippingRange(i,i+0.1);
      renderWindow->Render();
  }
  */

  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);
  interactor->Start();



  //vtkNew<vtkWindowToImageFilter> windowToImageFilter;
  //windowToImageFilter->SetInput(renderWindow);
  //windowToImageFilter->Update();

  //vtkNew<vtkPNGWriter> writer;
  //writer->SetFileName("test.png");
  //writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  //writer->Write();

  return EXIT_SUCCESS;
}
