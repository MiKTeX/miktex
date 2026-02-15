// Enum class for v3dheadertypes
// AUTO-GENERATED from v3dheadertypes.csv
namespace camp
{
enum v3dheadertypes : uint32_t
{
// UINT  Canvas width
canvasWidth=1,

// UINT  Canvas heighot
canvasHeight=2,

// BOOL  true: absolute size; false: scale to canvas
absolute=3,

// TRIPLE  Scene minimum bounding box corners
minBound=4,

// TRIPLE  Scene maximum bounding box corners
maxBound=5,

// BOOL  true: orthographic; false: perspective
orthographic=6,

// REAL  Field of view angle (in radians)
angleOfView=7,

// REAL  Initial zoom
initialZoom=8,

// PAIR  Viewport shift (for perspective projection)
viewportShift=9,

// PAIR  Margin around viewport
viewportMargin=10,

// RGB  Direction and color of each point light source
light=11,

// RGBA  Background color
background=12,

// REAL  Zoom base factor
zoomFactor=13,

// REAL  Zoom pinch factor
zoomPinchFactor=14,

// REAL  Zoom pinch limit
zoomPinchCap=15,

// REAL  Zoom power step
zoomStep=16,

// REAL  Shift-mode maximum hold distance (pixels)
shiftHoldDistance=17,

// REAL  Shift-mode hold time (milliseconds)
shiftWaitTime=18,

// REAL  Shift-mode vibrate time (milliseconds)
vibrateTime=19,

};

} // namespace camp
// End of File
