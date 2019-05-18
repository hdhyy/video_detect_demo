for %%i in (*.jpg) do start main.exe deploy_inference.prototxt pelee_SSD_304x304_map78.caffemodel "%%i"
pause