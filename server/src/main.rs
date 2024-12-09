use opencv::{
    core::{Point, Point_, Scalar, Vector},
    highgui, imgproc,
    objdetect::QRCodeDetector,
    prelude::*,
    videoio::{self, VideoCapture, CAP_ANY},
};

use opencv::core::BORDER_CONSTANT;

fn draw_bounding_box(frame: &mut Mat, bbox: &Mat) -> opencv::Result<()> {
    let p1 = Point::new(
        *bbox.at_2d::<f32>(0, 0)? as i32,
        *bbox.at_2d::<f32>(0, 1)? as i32,
    );
    let p2 = Point::new(
        *bbox.at_2d::<f32>(0, 2)? as i32,
        *bbox.at_2d::<f32>(0, 3)? as i32,
    );
    let p3 = Point::new(
        *bbox.at_2d::<f32>(0, 4)? as i32,
        *bbox.at_2d::<f32>(0, 5)? as i32,
    );
    let p4 = Point::new(
        *bbox.at_2d::<f32>(0, 6)? as i32,
        *bbox.at_2d::<f32>(0, 7)? as i32,
    );

    imgproc::line(
        frame,
        p1,
        p2,
        Scalar::new(255.0, 0.0, 0.0, 0.0),
        3,
        imgproc::LINE_8,
        0,
    )?;
    imgproc::line(
        frame,
        p2,
        p3,
        Scalar::new(0.0, 255.0, 0.0, 0.0),
        3,
        imgproc::LINE_8,
        0,
    )?;
    imgproc::line(
        frame,
        p3,
        p4,
        Scalar::new(0.0, 0.0, 255.0, 0.0),
        3,
        imgproc::LINE_8,
        0,
    )?;
    imgproc::line(
        frame,
        p4,
        p1,
        Scalar::new(255.0, 0.0, 255.0, 0.0),
        3,
        imgproc::LINE_8,
        0,
    )?;

    Ok(())
}

fn main() -> opencv::Result<()> {
    let mut cap = VideoCapture::new(0, CAP_ANY)?;
    if !cap.is_opened()? {
        println!("Error: Unable to access the camera");
        return Ok(());
    }

    let mut qr_decoder = QRCodeDetector::default()?;

    loop {
        let mut frame = Mat::default();
        cap.read(&mut frame)?;

        if frame.empty() {
            println!("Error: No frame captured");
            break;
        }

        let mut hsv_frame = Mat::default();
        imgproc::cvt_color(&frame, &mut hsv_frame, imgproc::COLOR_BGR2HSV, 0)?;

        let lower_green = Scalar::new(30.0, 50.0, 50.0, 0.0);
        let upper_green = Scalar::new(85.0, 255.0, 255.0, 0.0);
        let mut mask = Mat::default();

        opencv::core::in_range(&hsv_frame, &lower_green, &upper_green, &mut mask)?;

        let kernel = Mat::ones(3, 3, opencv::core::CV_8U)?;
        imgproc::erode(
            &mask,
            &mut mask,
            &kernel,
            Point::new(-1, -1),
            2,
            BORDER_CONSTANT,
            Scalar::default(),
        )?;
        imgproc::dilate(
            &mask,
            &mut mask,
            &kernel,
            Point::new(-1, -1),
            2,
            BORDER_CONSTANT,
            Scalar::default(),
        )?;

        let mut contours = Vector::<Vector<Point>>::new();
        imgproc::find_contours(
            &mask,
            &mut contours,
            imgproc::RETR_TREE,
            imgproc::CHAIN_APPROX_SIMPLE,
            Point::default(),
        )?;

        for contour in &contours {
            let area = imgproc::contour_area(&contour, false)?;
            if area > 1000.0 {
                let mut center = Point_::<f32>::default();
                let mut radius = 0.0;
                imgproc::min_enclosing_circle(&contour, &mut center, &mut radius)?;

                let center = Point_::<i32>::new(center.x as i32, center.y as i32);

                imgproc::circle(
                    &mut frame,
                    center,
                    radius as i32,
                    Scalar::new(0.0, 255.0, 0.0, 0.0),
                    3,
                    imgproc::LINE_8,
                    0,
                )?;
                imgproc::circle(
                    &mut frame,
                    center,
                    5,
                    Scalar::new(0.0, 0.0, 255.0, 0.0),
                    -1,
                    imgproc::LINE_8,
                    0,
                )?;
                imgproc::put_text(
                    &mut frame,
                    "Tennis Ball",
                    center,
                    imgproc::FONT_HERSHEY_SIMPLEX,
                    0.5,
                    Scalar::new(255.0, 255.0, 255.0, 0.0),
                    2,
                    imgproc::LINE_8,
                    false,
                )?;
            }
        }

        let mut bbox = Mat::default();
        let mut r_img = Mat::default();
        let qr_data = qr_decoder.detect_and_decode(&frame, &mut bbox, &mut r_img)?;
        if !qr_data.is_empty() {
            println!("QR Code Data: {:?}", qr_data);
            draw_bounding_box(&mut frame, &bbox)?;
            imgproc::put_text(
                &mut frame,
                &String::from_utf8(qr_data).expect("utf8 valid"),
                Point::new(50, 50),
                imgproc::FONT_HERSHEY_SIMPLEX,
                1.0,
                Scalar::new(0.0, 255.0, 255.0, 0.0),
                2,
                imgproc::LINE_8,
                false,
            )?;
        }

        highgui::imshow("Robot", &frame)?;

        if highgui::wait_key(10)? == 'q' as i32 || highgui::wait_key(10)? == 27 {
            break;
        }
    }

    Ok(())
}
