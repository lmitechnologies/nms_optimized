// nms_module_optimized.cpp

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace py = pybind11;

// Function to compute IoU between two boxes
inline float compute_iou(float x1_a, float y1_a, float x2_a, float y2_a,
                                   float x1_b, float y1_b, float x2_b, float y2_b) {
    float xx1 = std::max(x1_a, x1_b);
    float yy1 = std::max(y1_a, y1_b);
    float xx2 = std::min(x2_a, x2_b);
    float yy2 = std::min(y2_a, y2_b);

    float w = std::max(0.0f, xx2 - xx1 + 1.0f);
    float h = std::max(0.0f, yy2 - yy1 + 1.0f);
    float inter = w * h;

    float area_a = (x2_a - x1_a + 1.0f) * (y2_a - y1_a + 1.0f);
    float area_b = (x2_b - x1_b + 1.0f) * (y2_b - y1_b + 1.0f);
    float min_area = std::min(area_a, area_b);

    // Avoid division by zero
    if (min_area <= 0.0f) {
        return 0.0f;
    }

    return inter / min_area;
}

std::vector<size_t> nms_largest_box_optimized(
    const std::vector<std::vector<float>>& boxes_input, float overlap_thresh = 0.5f) {
    std::vector<size_t> keep;

    if (boxes_input.empty()) {
        return keep;
    }

    size_t num_boxes = boxes_input.size();

    // Separate coordinates and compute areas
    std::vector<float> x1(num_boxes);
    std::vector<float> y1(num_boxes);
    std::vector<float> x2(num_boxes);
    std::vector<float> y2(num_boxes);
    std::vector<float> areas(num_boxes);

    for (size_t i = 0; i < num_boxes; ++i) {
        if (boxes_input[i].size() < 4) {
            throw std::invalid_argument("Each box must have at least four coordinates: [x1, y1, x2, y2].");
        }
        x1[i] = boxes_input[i][0];
        y1[i] = boxes_input[i][1];
        x2[i] = boxes_input[i][2];
        y2[i] = boxes_input[i][3];
        areas[i] = (x2[i] - x1[i] + 1) * (y2[i] - y1[i] + 1);
    }

    // Create a list of indices sorted by area (largest first)
    std::vector<int> indices(num_boxes);
    for (size_t i = 0; i < num_boxes; ++i) {
        indices[i] = static_cast<int>(i);
    }

    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        return areas[a] > areas[b];
    });

    // Perform NMS
    while (!indices.empty()) {
        int current = indices[0];
        keep.push_back(current);

        std::vector<int> remaining;
        remaining.reserve(indices.size() - 1); // Optimize memory allocation

        for (size_t i = 1; i < indices.size(); ++i) {
            int idx = indices[i];
            // Compute IoU based on the smaller box
            float overlap = compute_iou(x1[current], y1[current], x2[current], y2[current],
                                                 x1[idx], y1[idx], x2[idx], y2[idx]);
            // make sure keep the largest box
            if (overlap <= overlap_thresh) {
                if (areas[idx] > areas[current]) {
                    remaining.push_back(idx);
                }
            }
        }

        // Update the list of indices
        indices = std::move(remaining);
    }

    return keep;
}



PYBIND11_MODULE(nms_module_optimized, m) {
    m.doc() = "Optimized Non-Maximum Suppression (NMS) module implemented in C++";

    m.def("nms_bbox_area", &nms_largest_box_optimized, 
          py::arg("boxes"), 
          py::arg("overlap_thresh") = 0.5f,
          R"pbdoc(
        Perform optimized non-maximum suppression (NMS) on a list of boxes and return the largest boxes.

        :param boxes: A list of bounding boxes in the format [x1, y1, x2, y2].
        :param overlap_thresh: Threshold for overlap; boxes with IoU greater than this threshold will be suppressed.
        :return: A list of boxes that survive after NMS.
    )pbdoc");
}