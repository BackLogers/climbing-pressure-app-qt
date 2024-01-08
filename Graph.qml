import QtQuick
import Qt5Compat.GraphicalEffects

Item {

    id: root
    property string mainTitle: "Title"
    property double mainTitleSize: root.height*0.05
    property string titleX: "X"
    property string titleY: "Y"
    property double axisXTitleSize: root.height*0.04
    property double axisYTitleSize: root.width*0.02

    property double axisXValueSize: root.height*0.03
    property double axisYValueSize: root.width*0.015

    property string gridColor: "#808080"
    property string gridBgColor: "#0e0e0e"
    property string bgColor: "#1e1e1e"
    property string textColor: "#F0F0F0"
    property string plotColor: "#FF0000"
    property string type: "logGraph"




    property double minAxisX: 1
    property double maxAxisX: 20000
    property double minAxisY: -1
    property double maxAxisY: 1
    property double minValueX: 1
    property double maxValueX: 20000
    property double minValueY: -1
    property double maxValueY: 1


    property int divNum: 10
    property var samples: new Array;

    Canvas {
        id: mainCanvas
        anchors.fill: root
        property double gridX: 0.07* root.width
        property double gridY: 0.15 * root.height
        property double gridW: root.width - 2*gridX
        property double gridH: root.height - 2*gridY
        onPaint: {


            let ctx = getContext("2d");
            ctx.reset()
            ctx.beginPath();

            //Background
            ctx.fillStyle = bgColor;
            ctx.fillRect(0, 0, width, height)

            ctx.textAlign="center";
            ctx.textBaseline="middle";
            ctx.fillStyle = textColor;
            ctx.font = root.mainTitleSize+"px sans-serif";
            ctx.fillText(mainTitle, root.width/2, gridY/2);

            ctx.font = root.axisXTitleSize+"px sans-serif";
            ctx.fillText(titleX, root.width/2, root.height-gridY*0.5);

            ctx.rotate(-Math.PI/2)
            ctx.font = root.axisYTitleSize+"px sans-serif";
            ctx.fillText(titleY,-root.height/2, gridX/2-root.axisXTitleSize/2)
            ctx.rotate(Math.PI/2)

            ctx.font = root.axisXValueSize+"px sans-serif";
            if(type==="logGraph"){

                let dl = Math.round(Math.pow(10, (Math.log10(minAxisX))))
                dl = dl===0?1:dl
                let dx = gridW/(Math.log10(maxAxisX)-Math.log10(minAxisX))
                let dy = gridH/divNum

                for(let i=minAxisX; i<maxAxisX; i+=dl){
                    let x=gridX+((Math.log10(i)-Math.log10(minAxisX))* dx)
                    if((i % (dl*10)) == 0) {
                        dl*=10
                        ctx.fillText(Math.round(dl),x, gridY+gridH+root.axisXValueSize);
                    }
                }

            }
            else{
                let val = minAxisX
                for(let x=gridX+gridW/divNum; x<gridX+gridW; x+=gridW/divNum){
                    val+=((maxAxisX-minAxisX)/divNum)
                        ctx.fillText(Math.round(val*100)/100,x, gridY+gridH+root.axisXValueSize);
                }
            }

            ctx.fillText(minAxisX,gridX, gridY+gridH+root.axisXValueSize);
            ctx.fillText(maxAxisX,gridX+gridW, gridY+gridH+root.axisXValueSize);

            ctx.font = root.axisYValueSize+"px sans-serif";
            ctx.textAlign="right";
            for(let k=0, l=maxAxisY; k<=divNum; k++, l-=(maxAxisY-minAxisY)/divNum)
                ctx.fillText(Math.round(l*100)/100,gridX-5, gridY + k*gridH/divNum);

            ctx.closePath()

        }

        Canvas{
            id:gridCanvas
            x: mainCanvas.gridX
            y: mainCanvas.gridY
            width: mainCanvas.gridW
            height: mainCanvas.gridH
            antialiasing: false

            onPaint: {
                let ctx = getContext("2d");

                let x = gridCanvas.x+2
                let y = gridCanvas.y+2
                let width = gridCanvas.width-4
                let height = gridCanvas.height-4
                ctx.reset()
                ctx.beginPath()
                ctx.fillStyle = gridBgColor;
                ctx.fillRect(0, 0, width, height)
                ctx.strokeStyle = gridColor
                ctx.lineWidth = 1

                if(type==="logGraph"){
                    let dl = Math.round(Math.pow(10, (Math.log10(minAxisX||1))))
                    let dx = width/(Math.log10(maxAxisX)-Math.log10(minAxisX||1))
                    let dy = height/divNum

                    for(let i=(minAxisX||1)+dl; i<maxAxisX; i+=dl){
                        let q=((Math.log10(i)-Math.log10(minAxisX||1))* dx)
                        ctx.moveTo(q, 0)
                        ctx.lineTo( q, height)
                        if((i % (dl*10)) == 0) {
                            dl*=10
                        }
                    }

                }
                else{
                    for(let i=width/divNum; i<width;i+=width/divNum){
                        ctx.moveTo(i, 0)
                        ctx.lineTo( i, height)
                    }
                }

                for(let i=0, l=maxAxisY; i<divNum; i++, l-=(maxAxisY-minAxisY)/divNum) {
                    ctx.moveTo(0, i*height/divNum )
                    ctx.lineTo(width,  i*height/divNum)
                }
                ctx.moveTo(width, 0)
                ctx.lineTo( width, height)
                ctx.moveTo(0, 0)
                ctx.lineTo( 0, height)
                ctx.moveTo(0, height )
                ctx.lineTo(width, height)
                ctx.stroke();
                ctx.beginPath()

                ctx.strokeStyle = plotColor
                ctx.lineWidth = 1
                if(type==="logGraph"){
                    let dy=height/(maxValueY-minValueY)
                    let dx = width/(Math.log10(maxValueX)-Math.log10(minValueX||1))
                    let gmy = height+dy*minValueY
                    for(let i=0; i<samples.length; i++){
                        if(minValueX>=0 && maxValueX <=samples[i].length) {
                            for(let f = minValueX||1; f < maxValueX-1; f++) {
                               ctx.moveTo(2+(Math.log10(f)-Math.log10(minValueX||1))*dx, gmy-(samples[i][f]*dy))
                               ctx.lineTo(2+(Math.log10(f+1)-Math.log10(minValueX||1))*dx, gmy-(samples[i][f+1]*dy))
                           }
                        }
                    }
                }
                else{
                    let dy = height/(maxValueY-minValueY)
                    let dx = width/(maxValueX-minValueX)
                    let gmy = height+dy*minValueY
                    for(let i=0; i<samples.length; i++){
                        if(minValueX>=0 && samples[i].length<=maxValueX) {
                            for(let f = minValueX; f < maxValueX-1; f++) {
                               ctx.moveTo(2+(f-minValueX)*dx, gmy-(samples[i][f]*dy))
                               ctx.lineTo(2+(f+1-minValueX)*dx, gmy-(samples[i][f+1]*dy))
                           }
                        }
                    }
                }

                ctx.stroke()
            }

        }




    }
    function dataUpdate(x) {
        root.samples = x;
        gridCanvas.requestPaint()

    }

    function clear(){
        root.samples.length = 0;
        gridCanvas.requestPaint()
    }
}
