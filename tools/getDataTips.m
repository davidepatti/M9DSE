function [content, dtHandle] = getDataTips(h, outputFormat)
%GETDATATIPS returns content and handles to existing data tips.
%   GETDATATIPS() prints the content to all existing data tips in the
%   current figure (gcf) to the command window.
%
%   GETDATATIPS(H) prints the content to all existing data that belong to
%   figures or axes listed in H which is an array of figure or axis
%   handles.
%
%   CONTENT = GETDATATIPS() returns a 1x1 cell of data tip content from the
%   current figure. CONTENT{1} is an nx1 cell array for n data tips in the
%   current figure.  CONTENT{1}{j} is an mx1 cell array of character
%   vectors from m-rows of the j-th data tip in the current figure.
%
%   CONTENT = GETDATATIPS(H) returns a cell array of data tip content with
%   the same shape as the input H. Similar to above, CONTENT{i}{j}{k}
%   returns a character vector from the k-th row of the j-th data tip from
%   handle H(i).
%
%   CONTENT = GETDATATIPS(H, OUTPUTFORMAT) combines the rows of data tip
%   strings into a character array when OUTPUTFORMAT is 'char' so that
%   CONTENT{i}{j} is an mxc character array of the data tip content rather
%   than an mx1 cell array which is produced whffen OUTPUTFORMAT is 'cell'.
%
%   [CONTENT, DTHANDLE] = GETDATATIPS(__) returns a cell array the same
%   shape as CONTENT containing handles to the data tips.  DTHANDLE{i} is 
%   an nx2 graphics array for the n data tips in the i-th figure or axis 
%   handle provided in the first input (or gcf when 1st input is missing).
%   DTHANDLE{i}(j,1) is the PointDataTip handle to the j-th data tip. 
%   DTHANDLE{i}(j,2) is the DataTip handle to the j-th data tip in newer 
%   Matlab releases or is a graphics placeholder in older releases.  
%
%   Example
%     % Set up figure & data tips
%     figure()
%     p = scatter(-3:.1:3, sin(-3:.1:3));
%
%     datatip(p,-3, sin(-3)); % datatip() requires Matlab r2019b or later
%     datatip(p, 0, 0);
%     datatip(p,1.5, sin(1.5));
%
%     getDataTips()          % print all datatips in current figure.
%     [c,h] = getDataTips()  % return datatip content & handles.
%
%   See getDataTips_examples.mlx for examples of all syntaxes. 
%
%   See also DATATIP, DATACURSORMODE, getCursorInfo.
%
% Source: <a href = "https://www.mathworks.com/matlabcentral/fileexchange/82038-getdatatips-get-data-tip-text-and-handles">getDataTips</a>
% Author: <a href = "https://www.mathworks.com/matlabcentral/profile/authors/3753776-adam-danz">Adam Danz</a> 
% Copyright (c) 2020  All rights reserved
% Version History
% vs 1.0.0   201029   Initial update to FEX.
% vs 1.0.1   201029   Added hyperlinks to doc and license info.
%% Input validity
narginchk(0,2)
assert(~verLessThan('Matlab', '8.4'), 'GETDTCONTENT:OldeRelease', ...
    '%s() is not supported in Matlab r2014a or earlier.', mfilename) % (see [3])
if nargin==0 || isempty(h)
    h = gcf();
else
    assert(all(ishghandle(h)), 'GETDTCONTENT:InvalidHandles', ...
        'H must be vector of valid graphics handles to figures or axes.')
end
if nargin < 2 || isempty(outputFormat)
    outputFormat = 'cell';
else
    validatestring(outputFormat,{'cell','char'}, mfilename, 'outputFormat');
end
nout = nargout();
%% Extract datatip content
OUT.content = cell(size(h));
OUT.dtHandle = OUT.content;
for i = 1:numel(h)
    [OUT.content{i}, OUT.dtHandle{i}] = getDT(h(i), outputFormat, nout);
end
% Assign outputs
if nout>0
    content = OUT.content; 
    dtHandle = OUT.dtHandle; 
end
function [dtContent, pdth] = getDT(handle, outputFormat, nout)
% HANDLE is a scalar graphics handle (figure or axes)
% OUTPUTFORMAT is either 'char' or 'cell'.
% NOUT is number of output args to main func.
% DTCONTENT is a cell array of all data tip content within HANDLE.
% PDTH is an nx2 array of handles to n data tips in 'handles'. Column
%   1 are PointDataTip handles. Column 2 are DataTip handles in newer
%   releases of Matlab or graphics placeholders in older releases.  
% Get figure handle for h
fig = ancestor(handle,'figure');
% Is the input handle the fig handle?
isFigHandle = isequal(fig, handle);
% Get all pointDataTip handles in figure (see [1,4])
figObjs = findall(fig);
pdth = figObjs(arrayfun(@(h)isa(h,'matlab.graphics.shape.internal.PointDataTip'),figObjs)); 
% Get all datatip handles in figure (see [1,4])
dth = findall(fig, 'Type', 'DataTip'); 
if isempty(dth)
    pdth(:,2) = gobjects(size(pdth));
else
    pdth(:,2) = dth;
end
% Remove any datatip handles that do not belong to input axes (if handle refers axes) (see [2])
if ~isempty(pdth)
    selectedAncestor = ismember(pdth(:,1), findall(handle)); 
    pdth(~selectedAncestor,:) = [];
end
% Display content of all data tips separated by a new line for each data tip (see [1])
if isempty(pdth)
    dtContent = {};
elseif isprop(pdth(1,2),'Content') 
    % for DataTip objs
    dtContent = {pdth(:,2).Content}'; % col vec
else
    % for PointDataTip objs, we want to access the string property. In some releases there is 
    % an interpreter property that needs to temporarily be set to 'none'.
    if isprop(pdth(1,1), 'Interpreter')
        origInterps = get(pdth(:,1), 'Interpreter');
        set(pdth(:,1), 'Interpreter', 'none');
    end
    if size(pdth,1)==1
        dtContent = cellstr({get(pdth(:,1),'String')});
    else
        dtContent = cellfun(@cellstr,get(pdth(:,1),'String'),'UniformOutput',false); % col vec
    end
    if isprop(pdth(1,1), 'Interpreter')
        set(pdth(:,1), {'Interpreter'}, origInterps);
    end
end
if nout==0
    % Print results to command window if there are no outputs
    figLink = ['<a href="matlab: figure(',num2str(fig.Number),') ">',sprintf('Figure %d',fig.Number),'</a>'];
    if isFigHandle
        addendum = '';
    else
        addendum = 'specified axes within';
    end
    if ~isempty(pdth)
        dtArray = reshape([dtContent,repelem({'------'},size(pdth,1),1)]',[],1);
        dtArrayCol = vertcat(dtArray{:});
        fprintf('Data tips found in %s %s:\n',addendum,figLink)
        disp(char(strcat(repmat({'   '},numel(dtArrayCol),1), dtArrayCol)))
    else
        fprintf('No data tips found in %s %s.\n',addendum,figLink)
    end
else
    % Format data tip content
    if strcmpi(outputFormat, 'char')
        dtContent = cellfun(@char,dtContent,'UniformOutput',false);
    end
end
%% Footnotes
% [1] For Matlab r2018b (I think) and later, data tips produce 'DataTip' and 'PointDataTip'
%   objects but only the latter are returned for earlier releases.  DataTip objs have a 
%   'Content' property containing the data tip string but PointDataTip objs have a 'String'
%   property containing the data tip in the data tip's interpreter format. When the 
%   interpreter is converted to 'none', the PointDataTip string should match the DataTip
%   content. This function uses the DataTip method when available or the PointDataTip, 
%   otherwise. 
% [2] 'handle' can be a handle to a figure or axes and there are many types of axes. This
%   method searches for membership of data tip handles to the 'handles' input whether
%   'handles' refers to a figure or axes no matter what axes type. If 'handles' is not a
%   figure or axes, due to user error, there will not be a membership.
% [3] Data tips in Matlab r2014a and earlier will not be identified by the methods herein. 
% [4] The parent for DataTips is the graphics object the data tip is assigned to whereas
%   the parent for PointDataTips is the axes or group. 
% Another way to get some of the data tip content is:
%       datacursormode on
%       dcmObj = datacursormode(gcf);
%       dcmStruc = getCursorInfo(dcmObj);