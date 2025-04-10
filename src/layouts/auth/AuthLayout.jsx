import { Outlet, Link } from "react-router-dom";
import "./AuthLayout.css"; // Make sure this import is correct

const AuthLayout = () => {
  return (
    <div>
      <div className="logo-wrapper">
        <Link to="/">
          <img className="logo" src="logo.png" alt="Logo" />
        </Link>
      </div>
      <Outlet />
    </div>
  );
};

export default AuthLayout;
