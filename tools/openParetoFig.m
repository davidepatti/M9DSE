function openParetoFig(pareto_exp_filename, figureHandle, padding)
    if exist('padding', 'var') == 0
        padding = 4;
    end
    
    try
        if(isnumeric(figureHandle))
            index = figureHandle;
        else
            [content, dtHandles] = getDataTips(figureHandle);
            index = str2num(content{1}{1}{4}(7:end))
        end
    catch
        index = 1;
    end
    
    index_str = num2str(index, "%0" + padding + "d");
    
    [fpath,fname,fext] = fileparts(pareto_exp_filename);
    
    fig = openfig("Simula_results" + filesep + fname + fext + filesep + "SiC_Turn_ON_Modeling_" + fname + fext + "_" + index_str + ".fig");
    fig.Name =  "Pareto Conf #" + index_str;
end

