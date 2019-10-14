$(document).ready(function() {

    target = [
        1.009417120146360,
        0.463509966667219,
        19.837957528039961
    ];

    getTargetValues = function () {
        $("#Tvgs").val(target[0]);
        $("#Tvds").val(target[1]);
        $("#Tid").val(target[2]);
    };

    setTargetValues = function () {
        target[0] = $("#Tvgs").val();
        target[1] = $("#Tvds").val();
        target[2] = $("#Tid").val();
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
            { "data": "VGS" },
            { "data": "VDS" },
            { "data": "ID" }
        ],
        "order": [[1, 'asc']]
    } );

    $('#maxVgs, #maxVds, #maxId').keyup( function() {
        table.draw();
    } );

    $.fn.dataTable.ext.search.push(
        function( settings, data, dataIndex ) {
            var maxVgs = parseFloat($('#maxVgs').val());
            var maxVds = parseFloat($('#maxVds').val());
            var maxId  = parseFloat($('#maxId').val() );
            var vgs    = parseFloat(data[0]) || 0;
            var vds    = parseFloat(data[1]) || 0;
            var id     = parseFloat(data[2]) || 0;

            if ( (isNaN(maxVgs) || vgs < (maxVgs)) && (isNaN(maxVds) || vds < (maxVds)) && (isNaN(maxId) || id < (maxId))) {
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
