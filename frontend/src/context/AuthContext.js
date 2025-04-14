import { createContext, useContext, useState, useEffect } from "react";

const AuthContext = createContext(null);

const AuthProvider = ({ children }) => {
  const [user, setUser] = useState(
    localStorage.getItem("user")
      ? JSON.parse(localStorage.getItem("user"))
      : null
  );
  const [token, setToken] = useState(localStorage.getItem("authToken"));
  const [isLoggedIn, setIsLoggedIn] = useState(!!token);

  useEffect(() => {
    if (token) {
      setIsLoggedIn(true);
      localStorage.setItem("authToken", token);
      if (user) {
        localStorage.setItem("user", JSON.stringify(user));
      }
    } else {
      setIsLoggedIn(false);
      setUser(null);
      localStorage.removeItem("authToken");
      localStorage.removeItem("user");
    }
  }, [token, user]);

  const login = (newToken, userData) => {
    setToken(newToken);
    setUser(userData);
  };

  const logout = () => {
    setToken(null);
    setUser(null);
  };

  return (
    <AuthContext.Provider
      value={{ isLoggedIn: isLoggedIn, user, token, setUser, login, logout }}
    >
      {children}
    </AuthContext.Provider>
  );
};

const useAuth = () => {
  return useContext(AuthContext);
};

export { AuthProvider, useAuth };
