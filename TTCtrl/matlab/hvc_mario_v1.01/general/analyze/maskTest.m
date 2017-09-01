function res=maskTest(x,y_ref,y,tol,varargin)
%maskTest compares 2 functions/curves
%maskTest(x,y_ref,y,tol,[fig]);
%x is common for both the ideal and the tested curve
%y_ref is the ideal curve
%y is the tested curve
%tol is the maximum distance between the two curve for each single point
%
%returns a struct with the following elements
% pass is either 1 or 0 depending if it it passed or not passed
% dist is an array indicating for each y point the distance to the
% reference curve
% the function will return as soon the test fails, i.e. dist > tol


fig=1;
for ii=1:2:nargin-4
    eval([varargin{ii},'=varargin{ii+1};']);
end;


x=x(:);
y=y(:);
y_ref=y_ref(:);
ampX=max(x)-min(x);
ampY=max(y)-min(y);
ampY_ref=max(y_ref)-min(y_ref);
d=tol;
x=x/ampX;
y=y/ampY;
y_ref=y_ref/ampY_ref;
t = 0:0.1:2*pi;
xCircle = tol*cos(t);
yCircle = tol*sin(t);

%plot(x,y,x,y_ref);


step=x(2)-x(1);
x_dist=floor(d/step);
len=length(x);

for ii=1:len,
    ii;
    idx=[ii-x_dist:ii+x_dist];
    idx=idx(find(idx > 0 & idx <= len));
    dist(ii)=min((y(idx)-y_ref(ii)).^2+(x(idx)-x(ii)).^2);
    if(dist(ii) > d^2) 
        figure(fig);
        plot(x,y,x,y_ref,xCircle+x(ii),yCircle+y(ii),[0,x(ii)],[0,y(ii)]);
        break; 
    end;
end;

ii;
len;
if ii>=len,
    res.pass=1;
    disp('pass');
else
    res.pass=0;
    disp('fail');
end
res.dist=dist;