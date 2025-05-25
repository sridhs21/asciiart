#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <future>

using namespace std;

class ASCIIConverter {
protected:
    const string SIMPLE_CHARS = " .:-=+*#%@";
    const string DETAILED_CHARS = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    const string BLOCKS_CHARS = " ░▒▓█";
    
    string ascii_chars;
    int output_width;
    bool preserve_aspect_ratio;
    bool use_threading;
    
public:
    enum CharacterSet {
        SIMPLE,
        DETAILED,
        BLOCKS
    };
    
    ASCIIConverter(CharacterSet charset = DETAILED, int width = 120, 
                   bool preserve_aspect = true, bool threading = true) 
        : output_width(width), preserve_aspect_ratio(preserve_aspect), use_threading(threading) {
        
        switch(charset) {
            case SIMPLE: ascii_chars = SIMPLE_CHARS; break;
            case DETAILED: ascii_chars = DETAILED_CHARS; break;
            case BLOCKS: ascii_chars = BLOCKS_CHARS; break;
        }
    }
    
    string convertImage(const string& imagePath) {
        auto start = chrono::high_resolution_clock::now();
        
        cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
        if (image.empty()) {
            throw runtime_error("Could not load image: " + imagePath);
        }
        
        cout << "Original image size: " << image.cols << "x" << image.rows << endl;
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cv::Mat blurred;
        cv::GaussianBlur(gray, blurred, cv::Size(3, 3), 0);
        
        int output_height;
        if (preserve_aspect_ratio) {
            double aspect_ratio = static_cast<double>(image.rows) / image.cols;
            output_height = static_cast<int>(output_width * aspect_ratio * 0.5); // 0.5 for character aspect ratio
        } else {
            output_height = output_width / 2;
        }
        
        cv::Mat resized;
        cv::resize(blurred, resized, cv::Size(output_width, output_height), 0, 0, cv::INTER_AREA);
        cout << "ASCII output size: " << output_width << "x" << output_height << endl;

        string result;
        if (use_threading && output_height > 100) {
            result = convertWithThreading(resized);
        } else {
            result = convertSequential(resized);
        }
        
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
        cout << "Conversion completed in: " << duration.count() << "ms" << endl;
        return result;
    }
    
private:
    string convertSequential(const cv::Mat& image) {
        string result;
        result.reserve(image.rows * (image.cols + 1));
        for (int y = 0; y < image.rows; y++) {
            for (int x = 0; x < image.cols; x++) {
                uchar pixel = image.at<uchar>(y, x);
                int char_index = (pixel * (ascii_chars.length() - 1)) / 255;
                result += ascii_chars[char_index];
            }
            result += '\n';
        }
        return result;
    }
    
    string convertWithThreading(const cv::Mat& image) {
        const int num_threads = thread::hardware_concurrency();
        const int rows_per_thread = image.rows / num_threads;
        vector<future<string>> futures;
        for (int t = 0; t < num_threads; t++) {
            int start_row = t * rows_per_thread;
            int end_row = (t == num_threads - 1) ? image.rows : (t + 1) * rows_per_thread;
            
            futures.push_back(async(launch::async, [this, &image, start_row, end_row]() {
                string chunk;
                chunk.reserve((end_row - start_row) * (image.cols + 1));
                for (int y = start_row; y < end_row; y++) {
                    for (int x = 0; x < image.cols; x++) {
                        uchar pixel = image.at<uchar>(y, x);
                        int char_index = (pixel * (ascii_chars.length() - 1)) / 255;
                        chunk += ascii_chars[char_index];
                    }
                    chunk += '\n';
                }
                return chunk;
            }));
        } 

        string result;
        for (auto& future : futures) {
            result += future.get();
        }
        
        return result;
    }
};

class AdvancedASCIIConverter : public ASCIIConverter {
public:
    AdvancedASCIIConverter(CharacterSet charset = DETAILED, int width = 120) 
        : ASCIIConverter(charset, width) {}
    
    string convertToColoredHTML(const string& imagePath) {
        cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
        if (image.empty()) {
            throw runtime_error("Could not load image: " + imagePath);
        }
        
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
        cv::Mat resized_color, resized_gray;
        int output_height = static_cast<int>(output_width * 0.5 * image.rows / image.cols);
        cv::resize(image, resized_color, cv::Size(output_width, output_height), 0, 0, cv::INTER_AREA);
        cv::resize(gray, resized_gray, cv::Size(output_width, output_height), 0, 0, cv::INTER_AREA);
        string html = "<pre style=\"font-family: monospace; line-height: 1.0; font-size: 6px;\">";
        
        for (int y = 0; y < resized_gray.rows; y++) {
            for (int x = 0; x < resized_gray.cols; x++) {
                uchar pixel = resized_gray.at<uchar>(y, x);
                cv::Vec3b color = resized_color.at<cv::Vec3b>(y, x);
                int char_index = (pixel * (ascii_chars.length() - 1)) / 255;
                char ascii_char = ascii_chars[char_index];
                
                html += "<span style=\"color: rgb(" + 
                       to_string(color[2]) + "," + 
                       to_string(color[1]) + "," + 
                       to_string(color[0]) + ");\">" + 
                       ascii_char + "</span>";
            }
            html += "\n";
        }
        
        html += "</pre>";
        return html;
    }
    

    void saveToFile(const string& ascii_art, const string& filename) {
        ofstream file(filename, ios::binary);
        if (!file) {
            throw runtime_error("Could not create output file: " + filename);
        }
        const size_t chunk_size = 8192;
        for (size_t i = 0; i < ascii_art.length(); i += chunk_size) {
            size_t len = min(chunk_size, ascii_art.length() - i);
            file.write(ascii_art.c_str() + i, len);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <image_path> [output_width] [charset]\n";
        cout << "Charset options: 0=SIMPLE, 1=DETAILED, 2=BLOCKS\n";
        return -1;
    }
    
    string image_path = argv[1];
    int width = (argc > 2) ? atoi(argv[2]) : 120;
    ASCIIConverter::CharacterSet charset = (argc > 3) ? 
        static_cast<ASCIIConverter::CharacterSet>(atoi(argv[3])) : 
        ASCIIConverter::DETAILED;
    
    try {
        ASCIIConverter converter(charset, width, true, true);
        string ascii_art = converter.convertImage(image_path);
        string output_filename = "output_ascii.txt";
        ofstream output_file(output_filename);
        output_file << ascii_art;
        output_file.close();
        cout << "ASCII art saved to: " << output_filename << endl;

        if (ascii_art.length() < 10000) {
            cout << "\nPreview:\n" << ascii_art.substr(0, 1000);
            if (ascii_art.length() > 1000) {
                cout << "\n... (truncated, see file for full output)\n";
            }
        }

        AdvancedASCIIConverter advanced_converter(charset, width);
        string colored_html = advanced_converter.convertToColoredHTML(image_path);
        advanced_converter.saveToFile(colored_html, "output_colored.html");
        cout << "Colored HTML version saved to: output_colored.html" << endl;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }
    return 0;
}