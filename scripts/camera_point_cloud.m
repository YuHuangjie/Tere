function POINTS = camera_point_cloud(cam_pose_file)
%% Syntax
% camera_point_cloud(cam_pose_file)

%% code

src_file_name = cam_pose_file;
src_file = fopen(src_file_name);

src = textscan(src_file, '%d %f %f %f %f %f %f %f %f %f', 'Delimiter', '\t', 'CommentStyle', '#');
src = [src{1, 8}, src{1, 9}, src{1, 10}];
N_CAM = size(src, 1);
fclose(src_file);

POINTS = zeros(N_CAM, 3);

for i = 1 : N_CAM
    POINTS(i, :) = [src(i, 1), src(i, 2), src(i, 3)];    
end

if exist('matlab_output', 'dir') ~= 7
    mkdir('matlab_output');
end

dst = 'matlab_output/point.xyz';
dst_file = fopen(dst, 'w');
for i = 1 : N_CAM
    fprintf(dst_file, '%f %f %f\r\n', POINTS(i, 1), POINTS(i, 2), POINTS(i, 3));
end
fclose(dst_file);