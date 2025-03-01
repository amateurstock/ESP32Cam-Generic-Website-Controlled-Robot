// Bad, bad javascript, but it works

var joyCanvas = document.getElementById("joyCanvas");
var joyCTX = joyCanvas.getContext("2d");
var joyDiv = document.getElementById("joyDiv");

var buttonCanvas = document.getElementById("buttonCanvas");
var buttonCTX = joyCanvas.getContext("2d");
var buttonContainer = document.getElementById("buttonContainer");

var joyHeight = joyDiv.offsetHeight;
var joyWidth = joyDiv.offsetWidth;
var joyMiddleHeight = joyHeight/2;
var joyMiddleWidth = joyWidth/2;

var buttonHeight = buttonContainer.offsetHeight;
var buttonWidth = buttonContainer.offsetWidth;

joyCanvas.style.background = "#ff8";
var joyBounds = joyCanvas.getBoundingClientRect();
buttonCanvas.style.background = "#ff8";
var buttonBounds = buttonCanvas.getBoundingClientRect();

var joyIntervalID = null;
var frameIntervalID = null;

var stream = document.getElementById("stream");
var baseHost;

// Mouse things
var mouseX = 0;
var mouseY = 0;
var dragFlag = false;
var thetaDeg = 0;

// Keyboard things
var keyVector = {x: 0, y: 0};
var listPressed = [];

// Output
var outX = 0;
var outY = 0;
var motorLeft = 0;
var motorRight = 0;

// Circle object
class circle {
    constructor (xpos, 
                 ypos, 
                 radius, 
                 lineColor, 
                 fill, 
                 fillColor) {
        this.xpos = xpos;
        this.ypos = ypos;
        this.radius = radius;
        this.lineColor = lineColor;
        this.fill = fill;
        this.fillColor = fillColor;
    }

    render(context) {
        context.beginPath();
        context.strokeStyle = this.lineColor;
        context.lineWidth = this.radius/10;
        context.arc(this.xpos,
                    this.ypos,
                    this.radius,
                    0,
                    Math.PI*2,
                    false);
        if (this.fill){
            context.fillStyle = this.fillColor;
            context.fill();
        }

        context.stroke();
    }
    
    // this only updates to middle of screen cuz i'm lazy af
    update() {
        this.xpos = joyMiddleWidth;
        this.ypos = joyMiddleHeight;
    }
}

var joyContainer = new circle(
        joyMiddleWidth,
        joyMiddleHeight,
        75,
        "black",
        true,
        "grey"
);
joyContainer.render(joyCTX);

var joyThumb = new circle(
        joyMiddleWidth,
        joyMiddleHeight,
        50,
        "black",
        true,
        "white"
);
joyThumb.render(joyCTX);

// Touch things
class touchCanvas {
    constructor(canvasID) {
        this.canvas = document.getElementById(canvasID);
        this.ctx = this.canvas.getContext("2d");
        this.activeTouches = {};
        updateVars("window"); joyCanvas.width = joyWidth;
        joyCanvas.height = joyHeight;

        if (canvasID === "joyCanvas") {
            this.canvas.addEventListener(
                "touchstart",
                this.joyStart.bind(this),
                { passive: false }
            );
            this.canvas.addEventListener(
                "touchmove",
                this.joyMove.bind(this),
                { passive: false }
            );
            this.canvas.addEventListener(
                "touchend",
                this.joyEnd.bind(this)
            );
        }

        else if (canvasID === "buttonCanvas") {
            this.canvas.addEventListener(
                "touchstart",
                this.buttonStart.bind(this),
                { passive: false }
            );
            this.canvas.addEventListener("touchend",
                this.buttonEnd.bind(this)
            );
        }

        else {
            while (true) {
                console.log("Error!", canvasID, "does not exist!");
                console.log("You fucking idiot!");
            }
        }
    }

    getTouchPos(touch) {
        const rect = this.canvas.getBoundingClientRect();
        return {
            x: touch.clientX - rect.left,
            y: touch.clientY - rect.top
        };
    }

    joyStart(event) {
        event.preventDefault();
        for (let touch of event.changedTouches) {
            const pos = this.getTouchPos(touch);
            this.activeTouches[touch.identifier] = pos;

            if (isNearTouch(pos.x, pos.y)) {
                dragFlag = true;
                thetaDeg = getAngle(
                    pos.x,
                    pos.y,
                    joyContainer.xpos,
                    joyContainer.ypos);

                joyThumb.xpos = pos.x;
                joyThumb.ypos = pos.y;

                joyThumb.fillColor = "blue";

                if (!isNearTouch(pos.x, pos.y)) {
                    limitThumb();
                }

                renderCircles();

                // not sure how this helps but whatever
                outX = Math.round(joyThumb.xpos - joyContainer.xpos);
                outY = Math.round(- joyThumb.ypos + joyContainer.ypos);
            }

            updateMotors(outX, outY, 255);
        }
    }

    joyMove(event) {
        event.preventDefault();
        for (let touch of event.changedTouches) {
            if (this.activeTouches[touch.identifier]) {
                const pos = this.getTouchPos(touch);
                this.activeTouches[touch.identifier] = pos;

                if (dragFlag) {
                    thetaDeg = getAngle(
                        pos.x,
                        pos.y,
                        joyContainer.xpos,
                        joyContainer.ypos);

                    joyThumb.xpos = pos.x;
                    joyThumb.ypos = pos.y;

                    joyThumb.fillColor = "blue";

                    if (!isNearTouch(pos.x, pos.y)) {
                        limitThumb();
                        joyThumb.fillColor = "magenta";
                    }
                    renderCircles();

                    outX = Math.round(joyThumb.xpos - joyContainer.xpos);
                    outY = Math.round(- joyThumb.ypos + joyContainer.ypos);
                }

                updateMotors(outX, outY, 255);
            }
        }
    }

    joyEnd(event) {
        for (let touch of event.changedTouches) {
            dragFlag = false;
            renderCircles("center");
            outX = 0;
            outY = 0;
            joyThumb.fillColor = "white";
            updateMotors(outX, outY, 255);

            delete this.activeTouches[touch.identifier];
        }
    }

    buttonStart(event) {
        event.preventDefault();
        for (let touch of event.changedTouches) {
            const pos = this.getTouchPos(touch);
            this.activeTouches[touch.identifier] = pos;
            console.log("Pressed.");
            buttonCanvas.style.background = "#ffb";
            const query = `${baseHost}/button?isPressing=1`;
            fetch(query)
                .then(response => {
                    console.log(`Inputs received! status: ${response.status}`);
                });
        }
    }

    buttonEnd(event) {
        for (let touch of event.changedTouches) {
            console.log("Released.");
            buttonCanvas.style.background = "#ff8";
            const query = `${baseHost}/button?isPressing=0`;
            fetch(query)
                .then(response => {
                    console.log(`Inputs received! status: ${response.status}`);
                });
        }
    }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++
// Functions ++++++++++++++++++++++++++++++++++++++++

function getAngle (x1, y1, x2, y2) {
    let x = -(x2 - x1);
    let y = y2 - y1;
    let inRads = Math.atan(y/x);
    let inDeg = 180 * inRads/ Math.PI;

    if (x <= 0) {
        inDeg += 180;
    }

    if (x > 0 && y < 0) {
        inDeg += 360;
    }

    return inDeg;
}

function updateVars (mode, e) {
    switch (mode) {
        case "window":
            joyHeight = joyDiv.offsetHeight;
            joyWidth = joyDiv.offsetWidth;
            joyMiddleHeight = joyHeight/2;
            joyMiddleWidth = joyWidth/2;
            break;

        case "mouse":
            joyBounds = joyCanvas.getBoundingClientRect();

            mouseX = e.clientX - joyBounds.left;
            mouseY = e.clientY - joyBounds.top;

            thetaDeg = getAngle(
                mouseX,
                mouseY,
                joyContainer.xpos,
                joyContainer.ypos);
            break;
    }
}

function renderCircles(mode) {
    joyCTX.clearRect(0, 0, joyWidth, joyHeight);
    if (mode === "center") {
        joyContainer.update();
        joyThumb.update();
        joyContainer.render(joyCTX);
        joyThumb.render(joyCTX);
    }

    else {
        joyContainer.render(joyCTX);
        joyThumb.render(joyCTX);
    }
}

function resize(e) {
    updateVars("window", e);
    updateVars("mouse", e);
    joyCanvas.width = joyWidth;
    joyCanvas.height = joyHeight;
    renderCircles("center");
}

function distance(a1, a2, b1, b2) {
    let inside = Math.pow(Math.abs(a1 - b1), 2) + 
                 Math.pow(Math.abs(a2 - b2), 2);
    return Math.sqrt(inside);
}

function isNear() {
    let dist = distance(
        mouseX,
        mouseY,
        joyMiddleWidth,
        joyMiddleHeight);

    if (dist < joyContainer.radius) return true;
    else return false;
}

function isNearTouch(touchX, touchY) {
    let dist = distance(
        touchX,
        touchY,
        joyMiddleWidth,
        joyMiddleHeight);

    if (dist < joyContainer.radius) return true;
    else return false;
}

function startDrawing() {
    if (isNear()) {
        dragFlag = true;
        joyThumb.fillColor = "blue";
        joyThumb.xpos = mouseX;
        joyThumb.ypos = mouseY;
    }

    renderCircles();

    outX = Math.round(joyThumb.xpos - joyContainer.xpos);
    outY = Math.round(- joyThumb.ypos + joyContainer.ypos);

    console.log(outX, outY);
}


function limitThumb() {
    let xFactor = Math.cos(thetaDeg * (Math.PI/180));
    let yFactor = Math.sin(thetaDeg * (Math.PI/180));

    let xComp = xFactor * (joyThumb.xpos - joyContainer.xpos);
    let yComp = -yFactor * (joyThumb.ypos - joyContainer.ypos);

    joyThumb.xpos = joyContainer.xpos + (xFactor * joyContainer.radius);
    joyThumb.ypos = joyContainer.ypos + (-yFactor * joyContainer.radius);

    outX = Math.round(joyThumb.xpos - joyContainer.xpos);
    outY = Math.round(- joyThumb.ypos + joyContainer.ypos);
}

function draw(e) {
    updateVars("mouse", e);
    if (isNear()) {
        joyThumb.fillColor = "cyan";
    } else {
        joyThumb.fillColor = "white";
    }

    if (dragFlag) {
        joyThumb.fillColor = "blue";
        joyThumb.xpos = mouseX;
        joyThumb.ypos = mouseY;

        if (!isNear()) {
            joyThumb.fillColor = "violet";
            limitThumb();
        }
    }

    renderCircles();

    outX = Math.round(joyThumb.xpos - joyContainer.xpos);
    outY = Math.round(- joyThumb.ypos + joyContainer.ypos);
}

function stopDrawing(e) {
    dragFlag = false;
    draw(e);
    renderCircles("center");
}

function updateMotors(x, y, maxVal) {
    motorLeft = Math.round ((255/75) * (y + x));
    motorRight = Math.round ((255/75) * (y - x));

    console.log ("Debug 1:", motorLeft, motorRight);

    motorLeft = Math.max(-maxVal, Math.min(maxVal, motorLeft));
    motorRight = Math.max(-maxVal, Math.min(maxVal, motorRight));

    console.log ("Debug 2:", motorLeft, motorRight);
}

function sendJoystick() {
    let leftVal = motorLeft;
    let rightVal = motorRight;

    const query = `${baseHost}/joystick?leftVal=${leftVal}&rightVal=${rightVal}`;
    fetch(query)
        .then(response => {
            console.log(`Inputs received! status: ${response.status}`);
        });
}

function stopSending() {
    if (joyIntervalID) {
        clearInterval(joyIntervalID);
        joyIntervalID = null;
        console.log("Stopped sending data.");
    }
}



function parseKeyVector(input, mode) {
    toProcess = input.toLowerCase();

    if (mode === "add") {
        if (listPressed.includes(toProcess)) {
            return;
        } else {
            listPressed.push(toProcess);
            return;
        }
    } else if (mode === "remove") {
        if (listPressed.includes(toProcess)) {
            listPressed = listPressed.filter(str => str !== toProcess);
            return
        } else {
            return;
        }
    } else {
        console.log(mode, "is not a mode lmao");
        return;
    }
}

function processPressed() {
    let dirVector = {
        up: 0,
        down: 0,
        left: 0,
        right: 0,
    }

    if (listPressed.includes("w")) dirVector.up = 255;
    if (listPressed.includes("s")) dirVector.down = 255;
    if (listPressed.includes("a")) dirVector.left = 255;
    if (listPressed.includes("d")) dirVector.right = 255;

    let resultant = {
        x: dirVector.up - dirVector.down,
        y: dirVector.right - dirVector.left
    }

    console.log(resultant);
}

// Functions ========================================
// ==================================================


// ++++++++++++++++++++++++++++++++++++++++++++++++++
// Event Listeners ++++++++++++++++++++++++++++++++++

window.addEventListener('resize', (e) => {
    resize(e);
});


// Mouse Events
document.addEventListener('mousedown', (e) => {
    startDrawing();

    updateMotors(outX, outY, 255);
    console.log(motorLeft);
});

document.addEventListener('mousemove', (e) => {
    draw(e);

    updateMotors(outX, outY, 255);
    console.log(motorLeft, motorRight);
});

document.addEventListener('mouseup', (e) => {
    stopDrawing(e);
    outX = 0;
    outY = 0;

    updateMotors(outX, outY, 255);
    console.log(motorLeft, motorRight);
});

document.addEventListener('keydown', function(event) {
    parseKeyVector(event.key, "add");
    processPressed();
    console.log(listPressed);
});

document.addEventListener('keyup', function(event) {
    parseKeyVector(event.key, "remove");
    processPressed();
    console.log(listPressed);
});

// Upon loading
document.addEventListener('DOMContentLoaded', () => {
    console.log("Things should settle down rn.");
    baseHost = document.location.origin;
    let streamURL = baseHost + ':81';
    stream.src = `${streamURL}/stream`;
    resize(event);
});


// Event Listeners ==================================
// ==================================================


new touchCanvas("joyCanvas");
new touchCanvas("buttonCanvas");

// Terrible patchwork but hey, it works

renderCircles("center");
//startSendingInputs(75);