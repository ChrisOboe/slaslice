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
#include <vtkFillHolesFilter.h>
#include <vtkCallbackCommand.h>
#include <vtkPolyDataNormals.h>
#include <vtkImageData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkImageStencilAlgorithm.h>
#include <vtkImageStencilSource.h>
#include <vtkPointData.h>
#include <vtkCleanPolyData.h>
#include <vtkAlgorithm.h>
#include <vtkCubeSource.h>
#include <vtkImageActor.h>
#include <vtkCollisionDetectionFilter.h>
#include <vtkImageStencilToImage.h>
#include <vtkTriangleFilter.h>
#include <vtkImageStencil.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkAxesActor.h>
#include <vtkDataSetMapper.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkImageProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkImageViewer2.h>
#include <vtkVolumeProperty.h>
#include <vtkImageReslice.h>
#include <array>


struct Printer {
    float Length; // mm
    float Width;
    float Height;
    int ResolutionX;
    int ResolutionY;
    int ResolutionZ;
    float BlockX(){return Length/ResolutionX;};
    float BlockY(){return Width/ResolutionY;};
    float BlockZ(){return Height/ResolutionZ;};
};

int main(int, char*[])
{
    Printer Saturn2 = Printer{
            .Length= 218.88,
            .Width=123.12,
            .Height=250,
            .ResolutionX=7680,
            .ResolutionY=4320,
            .ResolutionZ=10000,
    };


    vtkNew<vtkImageData> imageData;
    imageData->SetDimensions(1000,1000,1000);
    imageData->SetSpacing(1.0, 1.0, 1.0);
    imageData->SetOrigin(0.0, 0.0, 0.0);




    //vtkNew<vtkPoints> points;

  //vtkSmartPointer<vtkSTLReader> stlReader {vtkSmartPointer<vtkSTLReader>::New()};
  vtkNew<vtkSTLReader> stlReader;
  stlReader->SetFileName("/mnt/chump/3dDruck/sechsschwaenzer/Dispenser_with_Funnel.stl");
  stlReader->Update();


  auto polyData = stlReader->GetOutput();
double bounds[6];
polyData->GetBounds(bounds);


  if (polyData->GetNumberOfPoints() < 2 || polyData->GetNumberOfCells() < 2)
  {
    std::cout << "Convert: Cannot create binary labelmap from surface with number of points: "
        << polyData->GetNumberOfPoints() << " and number of cells: "
        << polyData->GetNumberOfCells() << std::endl;
  }

  // Compute polydata normals
  vtkNew<vtkPolyDataNormals> normalFilter;
  normalFilter->SetInputData(polyData);
  normalFilter->ConsistencyOn();

  // Make sure that we have a clean triangle polydata
  vtkNew<vtkTriangleFilter> triangle;
  triangle->SetInputConnection(normalFilter->GetOutputPort());

  // Convert to triangle strip
  vtkNew<vtkStripper> stripper;
  stripper->SetInputConnection(triangle->GetOutputPort());

  // Convert polydata to stencil
  vtkNew<vtkPolyDataToImageStencil> polyDataToImageStencil;
  polyDataToImageStencil->SetInputConnection(stripper->GetOutputPort());
  polyDataToImageStencil->SetOutputSpacing(imageData->GetSpacing());
  polyDataToImageStencil->SetOutputOrigin(bounds[0], bounds[2], bounds[4]);
  polyDataToImageStencil->SetOutputWholeExtent(imageData->GetExtent());
  polyDataToImageStencil->Update();

  // Convert stencil to image
  vtkNew<vtkImageStencilToImage> imageStencilToImage;
  imageStencilToImage->SetInputConnection(polyDataToImageStencil->GetOutputPort());
  imageStencilToImage->SetOutsideValue(0);
  imageStencilToImage->SetInsideValue(255);
  imageStencilToImage->SetOutput(imageData);
  imageStencilToImage->Update();


    vtkNew<vtkTransform> transform;
    transform->Translate(0,0,60);

  vtkNew<vtkImageReslice> reslice;
  reslice->SetOutputExtent(bounds[0],300,bounds[2],300,0,1);
  reslice->SetInputData(imageData);
  reslice->SetOutputSpacing(1,1,1);
  reslice->SetOutputOrigin(0,0,50);
  //reslice->SetResliceTransform(transform);
  reslice->Update();

  vtkNew<vtkImageViewer2> imageViewer;
    imageViewer->SetInputConnection(reslice->GetOutputPort());
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    imageViewer->SetupInteractor(renderWindowInteractor);;
    imageViewer->GetRenderWindow()->SetWindowName("ImageRotate");
    imageViewer->GetRenderer()->ResetCamera();
    imageViewer->Render();


    /*

    int* dims = imageData->GetDimensions();
    for (int z=0; z<dims[2]; z++)
    {
        reslice->SetOutputOrigin(0,0,z);
        reslice->Update();
        imageViewer->Render();
        std::cout << z << std::endl;
    }
    */

    renderWindowInteractor->Start();



    return EXIT_SUCCESS;


  /*
  vtkNew<vtkSmartVolumeMapper> volmapper;
  volmapper->SetInputData(imageData);

  vtkNew<vtkVolumeProperty> volProp;


  vtkNew<vtkVolume> volume;
  volume->SetMapper(volmapper);
  volume->SetProperty(volProp);







  vtkNew<vtkRenderer> renderer;
  renderer->AddVolume(volume);
  renderer->ResetCamera();

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(1920, 1080);
  //renderWindow->SetOffScreenRendering(true);
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("Slicer Preview");


  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);

    interactor->Start();
  /*








  //vtkSmartPointer<vtkImageData> id = PolyDataToImageData(cleanFilter->GetOutput(), 500);

  vtkNew<vtkDataSetMapper> imageMapper;
  imageMapper->SetInputData(id);

  vtkNew<vtkActor> imageActor;
  imageActor->SetMapper(imageMapper);
  imageActor->GetProperty()->SetColor(1,1,1);



  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(imageActor);
  renderer->ResetCamera();
  //renderer->GetActiveCamera()->SetParallelProjection(true);
  //renderer->GetActiveCamera()->SetFocalPoint(0,0,0);

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(1920, 1080);
  //renderWindow->SetOffScreenRendering(true);
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("Slicer Preview");

  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);


  vtkNew<vtkAxesActor> axes;
  vtkNew<vtkOrientationMarkerWidget> mw;

  mw->SetOrientationMarker(axes);
  mw->SetInteractor(interactor);
  mw->SetViewport(0.0,0.0,0.4,0.4);
  mw->SetEnabled(1);
  mw->InteractiveOn();

  interactor->Start();

  /*

  vtkNew<vtkCubeSource> cube;
  cube->SetXLength(Saturn2.BlockX());
  cube->SetYLength(Saturn2.BlockY());
  cube->SetZLength(Saturn2.BlockZ());


  vtkNew<vtkMatrix4x4> matrix1;
  vtkNew<vtkTransform> transform0;


  vtkNew<vtkCollisionDetectionFilter> collide;
  collide->SetInputConnection(0,stlReader->GetOutputPort());
  collide->SetTransform(0, transform0);
  collide->SetInputConnection(1,cube->GetOutputPort());
  collide->SetMatrix(1, matrix1);
  collide->SetBoxTolerance(0.0);
  collide->SetCellTolerance(0.0);
  collide->SetCollisionModeToAllContacts();
  collide->GenerateScalarsOn();

  for (int z=0; z<Saturn2.ResolutionZ; z++) {
      for (int y=0; y<Saturn2.ResolutionY; y++) {
          for (int x=0; x<Saturn2.ResolutionX; x++) {
              cube->SetCenter(x*Saturn2.BlockX(), y*Saturn2.BlockY(), z*Saturn2.BlockZ());
              cube->Update();
              collide->Update();
              if (collide->GetNumberOfContacts()>=1) {
                  points->InsertNextPoint(x,y,z);
              }


          }
      }
  }

  return EXIT_SUCCESS;


  // stl 1 == 1 mm

  /*


  vtkNew<vtkPlane> planea;
  planea->SetOrigin(0,0,10);
  planea->SetNormal(0, 0, -1);

  vtkNew<vtkCubeSource> cube;
  cube->SetYLength(200);
  cube->SetXLength(200);
  cube->SetZLength(200);
  cube->SetCenter(0,0,150);

  vtkNew<vtkCylinderSource> cyl;
  cyl->SetResolution(32);
  cyl->SetHeight(1);
  cyl->SetCenter(0,.5,0);

  vtkNew<vtkPolyDataBooleanFilter> slice;
  slice->SetInputConnection(0, stlReader->GetOutputPort());
  slice->SetInputConnection(1, cube->GetOutputPort());
  slice->SetOperModeToDifference();

  //slice->S
  slice->Update();

  vtkNew<vtkDataSetMapper> mapper;
  mapper->SetInputConnection(slice->GetOutputPort(0));


  vtkNew<vtkDataSetMapper> cmapper;
  cmapper->SetInputConnection(cube->GetOutputPort());
  vtkNew<vtkActor> cactor;
  cactor->GetProperty()->SetOpacity(.2);
  cactor->SetMapper(cmapper);

  vtkNew<vtkDataSetMapper> pmapper;
  pmapper->SetInputConnection(stlReader->GetOutputPort());
  vtkNew<vtkActor> pactor;
  pactor->GetProperty()->SetOpacity(.2);
  pactor->SetMapper(pmapper);

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  //actor->GetProperty()->SetColor(1,1,1);
  //actor->GetProperty()->SetAmbient(1);
  //actor->GetProperty()->SetDiffuse(0);
  //actor->GetProperty()->SetSpecular(0);


  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  //renderer->AddActor(cactor);
  //renderer->AddActor(pactor);
  renderer->ResetCamera();
  renderer->GetActiveCamera()->SetParallelProjection(true);
  renderer->GetActiveCamera()->SetFocalPoint(0,0,0);

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(1920, 1080);
  //renderWindow->SetOffScreenRendering(true);
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("Slicer Preview");

  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);
  interactor->Start();

  /*

  double bounds[6];
   stlReader->GetOutput()->GetBounds(bounds);
   double spacing[3]; // desired volume spacing
   spacing[0] = 0.05;
   spacing[1] = 0.05;
   spacing[2] = 0.05;

   int dim[3];
     for (int i = 0; i < 3; i++)
     {
       dim[i] = static_cast<int>(
           ceil((bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i]));
     }

     double origin[3];
     origin[0] = bounds[0] + spacing[0] / 2;
     origin[1] = bounds[2] + spacing[1] / 2;
     origin[2] = bounds[4] + spacing[2] / 2;


  vtkNew<vtkImageData> whiteImage;
  whiteImage->SetSpacing(spacing);
  whiteImage->SetDimensions(dim);
  whiteImage->SetExtent(0,dim[0]-1,0,dim[1]-1,0,dim[2]-1);
  whiteImage->SetOrigin(origin);
  whiteImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  vtkIdType count = whiteImage->GetNumberOfPoints();
  for (vtkIdType i = 0; i < count; ++i)
  {
    whiteImage->GetPointData()->GetScalars()->SetTuple1(i, 255);
  }

  vtkNew<vtkPolyDataToImageStencil> p2i;
  //p2i->SetEnableSMP(true);
  p2i->SetInputData(stlReader->GetOutput());
  p2i->SetOutputOrigin(whiteImage->GetOrigin());
  p2i->SetOutputSpacing(whiteImage->GetSpacing());
  p2i->SetOutputWholeExtent(whiteImage->GetExtent());
  p2i->Update();


  vtkNew<vtkImageStencil> imgstenc;
  imgstenc->SetInputData(whiteImage);
  imgstenc->SetStencilConnection(p2i->GetOutputPort());
  //imgstenc->ReverseStencilOff();
  imgstenc->SetBackgroundValue(0);
  imgstenc->Update();

  vtkNew<vtkDataSetMapper> imageMapper;
  imageMapper->SetInputConnection(imgstenc->GetOutputPort());

  vtkNew<vtkActor> imageActor;
  imageActor->SetMapper(imageMapper);
  imageActor->GetProperty()->SetColor(1,1,1);
  imageActor->GetProperty()->SetAmbient(1);
  imageActor->GetProperty()->SetDiffuse(0);
  imageActor->GetProperty()->SetSpecular(0);


  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(imageActor);
  renderer->ResetCamera();
  renderer->GetActiveCamera()->SetParallelProjection(true);
  renderer->GetActiveCamera()->SetFocalPoint(0,0,0);

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(1920, 1080);
  //renderWindow->SetOffScreenRendering(true);
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("Slicer Preview");

  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);
  interactor->Start();

  /*


  vtkNew<vtkPolyDataMapper> stlMapper;
  stlMapper->SetInputConnection(stlReader->GetOutputPort());

  vtkNew<vtkPlane> planea;
  planea->SetOrigin(0,0,10);
  planea->SetNormal(0, 0, -1);

  vtkNew<vtkPlane> planeb;
  planeb->SetOrigin(0,0,9.5);
  planeb->SetNormal(0, 0, 1);

  vtkNew<vtkClipPolyData> clippera;
  clippera->SetInputData(stlReader->GetOutput());
  clippera->SetClipFunction(planea);
  //clipper->GenerateClippedOutputOn();
  clippera->Update();

  vtkNew<vtkClipPolyData> clipper;
  clipper->SetInputData(clippera->GetOutput());
  clipper->SetClipFunction(planeb);
  //clipper->GenerateClippedOutputOn();
  clipper->Update();


  vtkNew<vtkDataSetMapper> clipMapper;
  clipMapper->SetInputData(clipper->GetOutput());

  vtkNew<vtkActor> clipActor;
  clipActor->SetMapper(clipMapper);
  clipActor->GetProperty()->SetColor(1,1,1);
  clipActor->GetProperty()->SetAmbient(1);
  clipActor->GetProperty()->SetDiffuse(0);
  clipActor->GetProperty()->SetSpecular(0);
  //clipActor->SetPosition(0,0,0);

  vtkNew<vtkProperty> backFaces;
  backFaces->SetSpecular(0);
  backFaces->SetDiffuse(0);
  backFaces->SetAmbient(1);
  backFaces->SetAmbientColor(1,1,1);
  clipActor->SetBackfaceProperty(backFaces);

  // The renderer generates the image
  // which is then displayed on the render window.
  // It can be thought of as a scene to which the actor is added
  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(clipActor);
  //renderer->AddActor(clippedAwayActor);
  //renderer->AddActor(boundaryActor);
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
      //boundaryStrips->Update();
      //renderer->GetActiveCamera()->SetClippingRange(i,i+0.1);
      renderWindow->Render();
  }
  */


  //vtkNew<vtkRenderWindowInteractor> interactor;
  //interactor->SetRenderWindow(renderWindow);
  //interactor->Start();



  //vtkNew<vtkWindowToImageFilter> windowToImageFilter;
  //windowToImageFilter->SetInput(renderWindow);
  //windowToImageFilter->Update();

  //vtkNew<vtkPNGWriter> writer;
  //writer->SetFileName("test.png");
  //writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  //writer->Write();

  return EXIT_SUCCESS;
}
