import numpy as np
import cv2

cap = cv2.VideoCapture(0)
#native resolution
print ("%d,%d\n" % (cap.get(3),cap.get(4)))
#ret = cap.set(3,1920)
#ret = cap.set(4,1080)
while(True):
  # Capture frame-by-frame
  ret, frame = cap.read()
  reframe = cv2.resize(frame, (0,0),fx=0.5, fy = 0.5)
  # Our operations on the frame come here
  gray = cv2.cvtColor(reframe, cv2.COLOR_BGR2GRAY)
  ret,thresh = cv2.threshold(gray,240,255,cv2.THRESH_BINARY)
  contours, hierarchy = cv2.findContours(thresh,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
  img = cv2.drawContours(gray,contours,-1,(0,0,255),3)
  lednum = len(contours)
  font = cv2.FONT_HERSHEY_SIMPLEX
  cv2.putText(gray,"%d"%lednum,(50,200), font, 4,(255,255,255),2)
# Display the resulting frame
  cv2.imshow('Ultim8x8',gray)
  if cv2.waitKey(1) & 0xFF == ord('q'):
     break
#When everything done, release the capture
cap.release()
cv2.destroyAllWindows()
                                    
