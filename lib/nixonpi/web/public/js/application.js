$(document).ready(function () {
    $('#power-toggle-button').toggleButtons({
        onChange:function ($el, status, e) {
            // $el = $('#toggle-button');
            // status = [true, false], the value of the checkbox
            // e = the event
            $el.closest("form").submit();
            console.log($el, status, e);
        },
        width:150,
        height:60,
        font:{
            'font-size':'40px'
        },
        animated:true,
        transitionspeed:0.5, // Accepted values float or "percent" [ 1, 0.5, "150%" ]
        label:{
            enabled:"I",
            disabled:"0"
        },
        style:{
            // Accepted values ["primary", "danger", "info", "success", "warning"] or nothing
            enabled:"success",
            disabled:"danger"
            /*custom:{
             enabled:{
             background:"#3584b8"
             gradient:"#D300D3",
             color:"#FFFFFF"
             },
             disabled:{
             background:"#FFAA00",
             gradient:"#DD9900",
             color:"#333333"

             }
             }*/
        }
    });

    $(":range").change(function (event, value) {
        console.info("value changed to", value);
        $(this).closest("form").submit();
    });

    $(".btn-lamp").click(function () {
        var checkBoxes = $(this).children("input[name=values\\[\\]]");
        checkBoxes.attr("checked", !checkBoxes.attr("checked"));
        $(this).closest("form").submit();
    });

    $(".chzn-select").chosen();

    //$("#form_field").trigger("liszt:updated");

    // Live watch events on form submit
    $(document).on('submit', 'form[data-remote]', function (e) {
        e.preventDefault();
        self = $(this);
        console.log("form sent with: " + self.serializeArray());
        $.ajax({
            url:self.attr('action'),
            data:self.serializeArray(),
            type:self.attr('method'),
            dataType:'json',
            success:function (res) {
                console.log(res)
            }
        })
    });

    $(":range").rangeinput({
        progress: true
    });

    /*
     var drop2 = $("select[name=drop2] option"); // the collection of initial options
     $("select[name=drop1]").change(function () {
     var drop1selected = parseInt(this.value); //get drop1 's selected value
     alert(drop1selected);
     $("select[name=drop2]").html(drop2).find('option').filter(function () {
     return parseInt(this.value) < drop1selected;
     }).remove();
     });*/

});