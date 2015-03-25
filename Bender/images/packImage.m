function packImage()

  pitch1 = 60;
 pitch2 = 66;
 for i=1:11
     fileNamer = sprintf('stucki%d_%d.bmp',pitch1,i);
     image = imread(fileNamer);
     imageName = sprintf('image%d.bmp',i);
     packImageWorker(image,image,image,imageName);
 end
 
  for i=1:11
     fileNamer = sprintf('stucki%d_%d.bmp',pitch2,i);
     image = imread(fileNamer);
     imageName = sprintf('image%d.bmp',i+11);
     packImageWorker(image,image,image,imageName);
 end



end



function rgbImage = packImageWorker(image1,image2,image3,outputname)
    [a,b] = size(image1);
    rgbImage = zeros(a,b,3);
    rgbImage(:,:,1) = image1;
    rgbImage(:,:,2) = image2;
    rgbImage(:,:,3) = image3;
    imwrite(rgbImage,outputname,'bmp');
    
    
end