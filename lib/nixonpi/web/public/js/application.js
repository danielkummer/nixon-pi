$(document).ready(function () {

    $('#power-toggle-button').toggleButtons({
        onChange:function ($el, status, e) {
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
            enabled:"success",
            disabled:"danger"
        }
    });


    $(".btn-lamp").click(function () {
        var $checkBoxes = $(this).children("input[name=values\\[\\]]");
        $checkBoxes.attr("checked", !$checkBoxes.attr("checked"));
        $(this).closest("form").submit();
    });


    //$("#form_field").trigger("liszt:updated");


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
                var text = res['message'] + " : " + res['value'];
                var $alert = $("<div class='alert alert-success fade in'><button data-dismiss='alert' class='close' type='button'>×</button>" + text + "</div>")
                $('#alert-container').append($alert);
                window.setTimeout(function () {
                    $alert.alert('close');
                }, 2000);
            },
            error:function (res, textStatus, errorMsg) {
                var $alert = $("<div class='alert alert-error fade in'><button data-dismiss='alert' class='close' type='button'>×</button>" + errorMsg + "</div>")
                $('#alert-container').append($alert);
            }
        })
    });

    $(":range").rangeinput({
        progress:true
    });


    $(":range").change(function (event, value) {
        console.info("value changed to", value);
        $(this).closest("form").submit();
    });

    $(".chzn-select").chosen().change(function (event) {
        var newValue = $(event.target).val();

        if (newValue == "cron") {
            $("#cron").show();
        } else {
            $("#cron").hide();
            $("#time").val("");
        }
    });

    $("#target").chosen().change(function (event) {
        var target = $(event.target).val();

        $.getJSON('command/' + target.toLowerCase() + '.json', function(data) {
          $('[name="command"]').val(JSON.stringify(data,null, 2))
        });

        if (target == "cron") {
            $("#cron").show();
        } else {
            $("#cron").hide();
            $("#time").val("");
        }
    });

    $("#cron").cron({
        onChange:function () {
            $("#time").val($(this).cron("value"));
        }
    });

    $('#time-defaults li a').click(function () {
        $('#time').val($(this).text());
    });

});