$(document).ready(function() {

    target = [
        1.009417120146360,   0.463509966667219,
        19.837957528039961, 29.069253843625720,
        0.602572120759444,   0.773710379993917
    ];

    getTargetValues = function () {
        $("#TvgsH").val(target[0]);
        $("#TvgsL").val(target[1]);
        $("#TvdsH").val(target[2]);
        $("#TvdsL").val(target[3]);
        $("#TidH").val(target[4]);
        $("#TidL").val(target[5]);
    };

    setTargetValues = function () {
        target[0] = $("#TvgsH").val();
        target[1] = $("#TvgsL").val();
        target[2] = $("#TvdsH").val();
        target[3] = $("#TvdsL").val();
        target[4] = $("#TidH").val();
        target[5] = $("#TidL").val();
        table.draw();
    };

    getTargetValues();

    set2target = function (fieldName, idx) {
        $("#"+fieldName).val(target[idx]);
        table.draw();
    };

    $.ajax({
        url: "./status.txt",
        success: function(status) {
            $("#current_simulation").html(status);
        }
    });

    var template;
    $.ajax({
        url: "./params.template",
        success: function(data) {
            template = data;
        }
    });   

    var table = $('#pareto').DataTable( {
        "ajax": "./pareto.latest.json",
        "columnDefs": [ {
            "targets": "_all",
            "createdCell": function (td, cellData, rowData, row, col) {
                if ( cellData > target[col-1] ) {
                    $(td).css('color', 'red');
                } else {
                    $(td).css('color', 'green');
                }
            }
        } ],
        "columns": [
            {
                "className": 'details-control',
                "orderable": false,
                "data": null,
                "defaultContent": ''
            },
            { "data": "VGS_H" },
            { "data": "VGS_L" },
            { "data": "VDS_H" },
            { "data": "VDS_L" },
            { "data": "ID_H" },
            { "data": "ID_L" }
        ],
        "order": [[1, 'asc']]
    } );

    $('#maxVgsH, #maxVgsL, #maxVdsH, #maxVdsL, #maxIdH, #maxIdL').keyup( function() {
        table.draw();
    } );

    $.fn.dataTable.ext.search.push(
        function( settings, data, dataIndex ) {
            var maxVgsH = parseFloat($('#maxVgsH').val());
            var maxVgsL = parseFloat($('#maxVgsL').val());
            var maxVdsH = parseFloat($('#maxVdsH').val());
            var maxVdsL = parseFloat($('#maxVdsL').val());
            var maxIdH  = parseFloat($('#maxIdH').val() );
            var maxIdL  = parseFloat($('#maxIdL').val() );
            var vgsH    = parseFloat(data[1]) || 0;
            var vgsL    = parseFloat(data[2]) || 0;
            var vdsH    = parseFloat(data[3]) || 0;
            var vdsL    = parseFloat(data[4]) || 0;
            var idH     = parseFloat(data[5]) || 0;
            var idL     = parseFloat(data[6]) || 0;

            if ( 
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && isNaN(maxVdsH) && isNaN(maxVdsL) && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && isNaN(maxVdsH) && isNaN(maxVdsL) && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && isNaN(maxVdsH) && isNaN(maxVdsL) && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && isNaN(maxVdsH) && isNaN(maxVdsL) && idH < maxIdH  && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && isNaN(maxVdsH) && vdsL <maxVdsL  && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && isNaN(maxVdsH) && vdsL <maxVdsL  && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && isNaN(maxVdsH) && vdsL <maxVdsL  && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && isNaN(maxVdsH) && vdsL <maxVdsL  && idH < maxIdH  && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && vdsH <maxVdsH  && isNaN(maxVdsL) && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && vdsH <maxVdsH  && isNaN(maxVdsL) && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && vdsH <maxVdsH  && isNaN(maxVdsL) && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && vdsH <maxVdsH  && isNaN(maxVdsL) && idH < maxIdH  && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && vdsH <maxVdsH  && vdsL <maxVdsL  && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && vdsH <maxVdsH  && vdsL <maxVdsL  && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && vdsH <maxVdsH  && vdsL <maxVdsL  && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && isNaN(maxVgsL) && vdsH <maxVdsH  && vdsL <maxVdsL  && idH < maxIdH  && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && isNaN(maxVdsH) && isNaN(maxVdsL) && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && isNaN(maxVdsH) && isNaN(maxVdsL) && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && isNaN(maxVdsH) && isNaN(maxVdsL) && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && isNaN(maxVdsH) && isNaN(maxVdsL) && idH < maxIdH  && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && isNaN(maxVdsH) && vdsL <maxVdsL  && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && isNaN(maxVdsH) && vdsL <maxVdsL  && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && isNaN(maxVdsH) && vdsL <maxVdsL  && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && isNaN(maxVdsH) && vdsL <maxVdsL  && idH < maxIdH  && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && vdsH <maxVdsH  && isNaN(maxVdsL) && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && vdsH <maxVdsH  && isNaN(maxVdsL) && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && vdsH <maxVdsH  && isNaN(maxVdsL) && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && vdsH <maxVdsH  && isNaN(maxVdsL) && idH < maxIdH  && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && vdsH <maxVdsH  && vdsL <maxVdsL  && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && vdsH <maxVdsH  && vdsL <maxVdsL  && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && vdsH <maxVdsH  && vdsL <maxVdsL  && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( isNaN(maxVgsH) && vgsL <maxVgsL  && vdsH <maxVdsH  && vdsL <maxVdsL  && idH < maxIdH  && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && isNaN(maxVdsH) && isNaN(maxVdsL) && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && isNaN(maxVdsH) && isNaN(maxVdsL) && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && isNaN(maxVdsH) && isNaN(maxVdsL) && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && isNaN(maxVdsH) && isNaN(maxVdsL) && idH < maxIdH  && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && isNaN(maxVdsH) && vdsL <maxVdsL  && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && isNaN(maxVdsH) && vdsL <maxVdsL  && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && isNaN(maxVdsH) && vdsL <maxVdsL  && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && isNaN(maxVdsH) && vdsL <maxVdsL  && idH < maxIdH  && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && vdsH <maxVdsH  && isNaN(maxVdsL) && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && vdsH <maxVdsH  && isNaN(maxVdsL) && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && vdsH <maxVdsH  && isNaN(maxVdsL) && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && vdsH <maxVdsH  && isNaN(maxVdsL) && idH < maxIdH  && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && vdsH <maxVdsH  && vdsL <maxVdsL  && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && vdsH <maxVdsH  && vdsL <maxVdsL  && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && vdsH <maxVdsH  && vdsL <maxVdsL  && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && isNaN(maxVgsL) && vdsH <maxVdsH  && vdsL <maxVdsL  && idH < maxIdH  && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && isNaN(maxVdsH) && isNaN(maxVdsL) && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && isNaN(maxVdsH) && isNaN(maxVdsL) && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && isNaN(maxVdsH) && isNaN(maxVdsL) && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && isNaN(maxVdsH) && isNaN(maxVdsL) && idH < maxIdH  && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && isNaN(maxVdsH) && vdsL <maxVdsL  && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && isNaN(maxVdsH) && vdsL <maxVdsL  && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && isNaN(maxVdsH) && vdsL <maxVdsL  && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && isNaN(maxVdsH) && vdsL <maxVdsL  && idH < maxIdH  && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && vdsH <maxVdsH  && isNaN(maxVdsL) && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && vdsH <maxVdsH  && isNaN(maxVdsL) && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && vdsH <maxVdsH  && isNaN(maxVdsL) && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && vdsH <maxVdsH  && isNaN(maxVdsL) && idH < maxIdH  && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && vdsH <maxVdsH  && vdsL <maxVdsL  && isNaN(maxIdH) && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && vdsH <maxVdsH  && vdsL <maxVdsL  && isNaN(maxIdH) && idL < maxIdL  ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && vdsH <maxVdsH  && vdsL <maxVdsL  && idH < maxIdH  && isNaN(maxIdL) ) ||
            ( vgsH <maxVgsH  && vgsL <maxVgsL  && vdsH <maxVdsH  && vdsL <maxVdsL  && idH < maxIdH  && idL < maxIdL  )
            ) {
                return true;
            }
            return false;
        }
    );

    function format (index, data) {
        var result = template;

        for (item in data) { 
            result = result.replace('$'+item, data[item]); 
        }

        return '<button type="button" id="copy_'+ index +'">Copy to clipboard</button><pre>' + result + '</pre>';
    };

    copyToClipboard = function (element) {
        var $temp = $("<textarea>");
        $("body").append($temp);
        $temp.val($(element).text()).select();
        document.execCommand("copy");
        $temp.remove();
    }

    $('#pareto tbody').on('click', 'td.details-control', function () {
        var tr = $(this).closest('tr');
        var row = table.row( tr );
 
        if ( row.child.isShown() ) {
            // This row is already open - close it
            row.child.hide();
            tr.removeClass('shown');
        }
        else {
            // Open this row
            row.child( format(row.index(), row.data())).show();
            $('#copy_'+row.index()).on('click', function(){copyToClipboard(this.nextElementSibling)});
            tr.addClass('shown');
        }
    } );

} );
