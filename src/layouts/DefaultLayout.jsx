import { Outlet } from "react-router-dom";
import Header from "../components/header/Header";

// made so that the header and sidebar don't show up when they're not necessary
const DefaultLayout = () => {
  return (
    <>
      <Header />
      <Outlet />
    </>
  );
};

export default DefaultLayout;
