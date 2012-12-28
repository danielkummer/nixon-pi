$('#power-toggle-button').toggleButtons({
    onChange:function ($el, status, e) {
        // $el = $('#toggle-button');
        // status = [true, false], the value of the checkbox
        // e = the event
        console.log($el, status, e);
    },
    width:100,
    height:25,
    font:{
        'font-size':'20px',
        'font-style':'italic'
    },
    animated:true,
    transitionspeed:1, // Accepted values float or "percent" [ 1, 0.5, "150%" ]
    label:{
        enabled:"ON",
        disabled:"OFF"
    },
    style:{
        // Accepted values ["primary", "danger", "info", "success", "warning"] or nothing
        enabled:"primary",
        disabled:"danger",
        custom:{
            enabled:{
                background:"#FF00FF",
                gradient:"#D300D3",
                color:"#FFFFFF"
            },
            disabled:{
                background:"#FFAA00",
                gradient:"#DD9900",
                color:"#333333"
            }
        }
    }
});


$(document).ready(function () {
    $(".btn-lamp").click(function () {
        console.log("clicked");
        var checkBoxes = $(this).children("input[name=values\\[\\]]");
        checkBoxes.attr("checked", !checkBoxes.attr("checked"));
    });
});