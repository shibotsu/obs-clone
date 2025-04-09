import { useEffect, useState } from "react";

function useResponsiveSlides() {
  const [itemsPerSlide, setItemsPerSlide] = useState(getItemsPerSlide());

  function getItemsPerSlide() {
    const width = window.innerWidth;
    if (width < 600) return 1;
    if (width < 900) return 2;
    if (width < 1200) return 3;
    return 4;
  }

  useEffect(() => {
    const handleResize = () => {
      setItemsPerSlide(getItemsPerSlide());
    };

    window.addEventListener("resize", handleResize);
    return () => window.removeEventListener("resize", handleResize);
  }, []);

  return itemsPerSlide;
}

export default useResponsiveSlides;
